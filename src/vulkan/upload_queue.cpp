#include "vulkan/upload_queue.hpp"
#include "vulkan/vulkan.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

	VulkanStagingBufferPool::VulkanStagingBufferPool(VulkanDevice* device) : device(device)
	{
		allBuffers.reserve(DefaultBufferCount);
		freeBuffers.reserve(DefaultBufferCount);
		for (size_t i = 0; i < DefaultBufferCount; ++i)
		{
			VulkanStagingBuffer buffer = CreateBuffer(DefaultBufferSize);
			allBuffers.push_back(buffer);
			freeBuffers.push_back(buffer);
		}
	}

	VulkanStagingBufferPool::~VulkanStagingBufferPool()
	{
		for (auto& buffer : allBuffers)
		{
			vmaDestroyBuffer(device->GetAllocator(), buffer.buffer, buffer.allocation);
		}
	}

	VulkanStagingBuffer VulkanStagingBufferPool::CreateBuffer(VkDeviceSize size) const
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VulkanStagingBuffer result = {};
		result.size = size;

		VmaAllocationInfo allocInfoOut;
		if (vmaCreateBuffer(device->GetAllocator(), &bufferInfo, &allocInfo, &result.buffer, &result.allocation, &allocInfoOut) != VK_SUCCESS)
		{
			return {};
		}
		result.mappedData = allocInfoOut.pMappedData;
		return result;
	}

	VulkanStagingBuffer VulkanStagingBufferPool::Rent(VkDeviceSize size)
	{
		std::lock_guard lock(mutex);

		for (size_t i = 0; i < freeBuffers.size(); ++i)
		{
			if (freeBuffers[i].size >= size)
			{
				VulkanStagingBuffer buffer = freeBuffers[i];
				freeBuffers.erase(freeBuffers.begin() + static_cast<ptrdiff_t>(i));
				return buffer;
			}
		}

		VulkanStagingBuffer buffer = CreateBuffer(size > DefaultBufferSize ? size : DefaultBufferSize);
		allBuffers.push_back(buffer);
		return buffer;
	}

	void VulkanStagingBufferPool::Return(const VulkanStagingBuffer& buffer)
	{
		std::lock_guard lock(mutex);

		if (freeBuffers.size() >= MaxFreeBufferCount)
		{
			vmaDestroyBuffer(device->GetAllocator(), buffer.buffer, buffer.allocation);
			auto it = std::find_if(allBuffers.begin(), allBuffers.end(), [&](const VulkanStagingBuffer& b) { return b.buffer == buffer.buffer; });
			if (it != allBuffers.end())
			{
				allBuffers.erase(it);
			}
			return;
		}

		freeBuffers.push_back(buffer);
	}

	VulkanUploadQueue::VulkanUploadQueue(VulkanDevice* device, const PrismObj<VulkanQueueStore>& transferQueueStore) : device(device), stagingPool(device), transferQueueStore(transferQueueStore)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = this->transferQueueStore->familyIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vkCreateCommandPool(device->GetDevice(), &poolInfo, device->GetAllocationCallbacks(), &commandPool);

		thread = std::thread(&VulkanUploadQueue::ThreadMain, this);
	}

	VulkanUploadQueue::~VulkanUploadQueue()
	{
		WaitIdle();

		running.store(false, std::memory_order_relaxed);
		pendingSignal.fetch_add(1, std::memory_order_release);
		pendingSignal.notify_one();
		if (thread.joinable())
		{
			thread.join();
		}

		for (auto& submission : freeSubmissions)
		{
			vkDestroyFence(device->GetDevice(), submission.fence, device->GetAllocationCallbacks());
			vkFreeCommandBuffers(device->GetDevice(), commandPool, 1, &submission.commandBuffer);
		}

		if (commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(device->GetDevice(), commandPool, device->GetAllocationCallbacks());
		}
	}

	void VulkanUploadQueue::Enqueue(VulkanUploadTask* task)
	{
		outstandingCount.fetch_add(1, std::memory_order_relaxed);
		{
			std::lock_guard lock(pendingMutex);
			pending.push(task);
		}
		pendingSignal.fetch_add(1, std::memory_order_release);
		pendingSignal.notify_one();
	}

	void VulkanUploadQueue::WaitIdle()
	{
		uint64_t current;
		while ((current = outstandingCount.load(std::memory_order_acquire)) != 0)
		{
			outstandingCount.wait(current, std::memory_order_relaxed);
		}
	}

	void VulkanUploadQueue::ThreadMain()
	{
		std::queue<VulkanUploadTask*> drained;
		while (true)
		{
			uint64_t seenSignal = pendingSignal.load(std::memory_order_acquire);

			{
				std::lock_guard lock(pendingMutex);
				std::swap(drained, pending);
			}

			while (!drained.empty())
			{
				SubmitTask(drained.front());
				drained.pop();
			}

			PollInFlight();

			if (!running.load(std::memory_order_relaxed))
			{
				return;
			}

			pendingSignal.wait(seenSignal, std::memory_order_relaxed);
		}
	}

	VulkanUploadQueue::UploadSubmission VulkanUploadQueue::AcquireSubmission()
	{
		if (!freeSubmissions.empty())
		{
			UploadSubmission submission = freeSubmissions.back();
			freeSubmissions.pop_back();
			vkResetFences(device->GetDevice(), 1, &submission.fence);
			vkResetCommandBuffer(submission.commandBuffer, 0);
			return submission;
		}

		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandPool = commandPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = 1;

		UploadSubmission submission = {};
		vkAllocateCommandBuffers(device->GetDevice(), &cmdAllocInfo, &submission.commandBuffer);

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vkCreateFence(device->GetDevice(), &fenceInfo, device->GetAllocationCallbacks(), &submission.fence);

		return submission;
	}

	void VulkanUploadQueue::ReleaseSubmission(UploadSubmission submission)
	{
		if (freeSubmissions.size() >= MaxFreeSubmissions)
		{
			vkDestroyFence(device->GetDevice(), submission.fence, device->GetAllocationCallbacks());
			vkFreeCommandBuffers(device->GetDevice(), commandPool, 1, &submission.commandBuffer);
			return;
		}

		freeSubmissions.push_back(submission);
	}

	namespace
	{
		void ReturnStagingBuffers(VulkanStagingBufferPool& pool, VulkanUploadTask* task)
		{
			if (auto* imageTask = dyn_cast<VulkanImageUploadTask>(task))
			{
				for (const auto& buffer : imageTask->stagingBuffers)
				{
					pool.Return(buffer);
				}
			}
			else
			{
				pool.Return(dyn_cast<VulkanBufferUploadTask>(task)->stagingBuffer);
			}
		}
	}

	void VulkanUploadQueue::SubmitTask(VulkanUploadTask* task)
	{
		UploadSubmission submission = AcquireSubmission();
		VkCommandBuffer cmd = submission.commandBuffer;

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmd, &beginInfo);

		if (auto* bufferTask = dyn_cast<VulkanBufferUploadTask>(task))
		{
			VkBufferCopy copyRegion = {};
			copyRegion.size = bufferTask->size;
			vkCmdCopyBuffer(cmd, bufferTask->stagingBuffer.buffer, bufferTask->buffer, 1, &copyRegion);

			vkEndCommandBuffer(cmd);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd;
			{
				std::lock_guard lock(transferQueueStore->mutex);
				vkQueueSubmit(transferQueueStore->queue, 1, &submitInfo, submission.fence);
			}

			inFlight.push_back({ task, submission });
			return;
		}

		auto& task_ = *dyn_cast<VulkanImageUploadTask>(task);

		auto barrier = [&](VkImageLayout oldLayout, VkImageLayout newLayout)
		{
			VkImageMemoryBarrier2 imageBarrier = {};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
			imageBarrier.oldLayout = oldLayout;
			imageBarrier.newLayout = newLayout;
			imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageBarrier.image = task_.image;
			imageBarrier.subresourceRange = { task_.aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };

			VkDependencyInfo depInfo = {};
			depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			depInfo.imageMemoryBarrierCount = 1;
			depInfo.pImageMemoryBarriers = &imageBarrier;
			vkCmdPipelineBarrier2(cmd, &depInfo);
		};

		barrier(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		for (size_t i = 0; i < task_.regions.size(); ++i)
		{
			const VulkanImageUploadRegion& region = task_.regions[i];
			VkBufferImageCopy copy = {};
			copy.bufferOffset = region.bufferOffset;
			copy.imageSubresource.aspectMask = task_.aspect;
			copy.imageSubresource.mipLevel = region.mipLevel;
			copy.imageSubresource.baseArrayLayer = region.baseArrayLayer;
			copy.imageSubresource.layerCount = 1;
			copy.imageExtent = { region.width, region.height, region.depth };

			vkCmdCopyBufferToImage(cmd, task_.stagingBuffers[i].buffer, task_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		}

		barrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, task_.finalLayout);

		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		{
			std::lock_guard lock(transferQueueStore->mutex);
			vkQueueSubmit(transferQueueStore->queue, 1, &submitInfo, submission.fence);
		}

		inFlight.push_back({ task, submission });
	}

	void VulkanUploadQueue::PollInFlight()
	{
		uint64_t count = inFlight.size();
		size_t batch = 0;
		for (size_t i = inFlight.size(); i-- > 0;)
		{
			uint64_t timeout = pollingRateMax / count;
			InFlightTask& entry = inFlight[i];
			if (vkWaitForFences(device->GetDevice(), 1, &entry.submission.fence, VK_TRUE, timeout) != VK_SUCCESS)
			{
				continue;
			}

			ReturnStagingBuffers(stagingPool, entry.task);
			if (WaitFlag* completionFlag = entry.task->completionFlag)
			{
				completionFlag->Signal();
			}

			ReleaseSubmission(entry.submission);

			inFlight.erase(inFlight.begin() + static_cast<ptrdiff_t>(i));
			++batch;
			--count;
		}

		if (batch > 0)
		{
			outstandingCount.fetch_sub(batch, std::memory_order_release);
			outstandingCount.notify_all();
		}
	}

HEXA_PRISM_NAMESPACE_END

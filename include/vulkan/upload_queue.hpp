#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class VulkanDevice;

struct VulkanStagingBuffer
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
	void* mappedData = nullptr;
	VkDeviceSize size = 0;
};

class VulkanStagingBufferPool
{
public:
	explicit VulkanStagingBufferPool(VulkanDevice* device);
	~VulkanStagingBufferPool();

	VulkanStagingBufferPool(const VulkanStagingBufferPool&) = delete;
	VulkanStagingBufferPool& operator=(const VulkanStagingBufferPool&) = delete;

	VulkanStagingBuffer Rent(VkDeviceSize size);
	void Return(const VulkanStagingBuffer& buffer);

private:
	VulkanStagingBuffer CreateBuffer(VkDeviceSize size) const;

	VulkanDevice* device;
	HEXA_UTILS_NAMESPACE::fmutex mutex; 
	std::vector<VulkanStagingBuffer> allBuffers;
	std::vector<VulkanStagingBuffer> freeBuffers;

	static constexpr VkDeviceSize DefaultBufferSize = 16ull * 1024 * 1024;
	static constexpr size_t DefaultBufferCount = 16;
	static constexpr size_t MaxFreeBufferCount = 32;
};

struct VulkanImageUploadRegion
{
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t width, height, depth;
	VkDeviceSize bufferOffset;
};

enum class VulkanUploadTaskType
{
	Invalid,
	Image,
	Buffer,
};

struct VulkanUploadTask
{
	WaitFlag* completionFlag = nullptr;
	VulkanUploadTaskType type = VulkanUploadTaskType::Invalid;
};

struct VulkanImageUploadTask : VulkanUploadTask
{
	static constexpr VulkanUploadTaskType Type = VulkanUploadTaskType::Image;
	VulkanImageUploadTask() { type = Type; }
	VkImage image;
	VkImageLayout finalLayout;
	VkImageAspectFlags aspect;
	std::vector<VulkanStagingBuffer> stagingBuffers;
	std::vector<VulkanImageUploadRegion> regions;
};

struct VulkanBufferUploadTask : VulkanUploadTask
{
	static constexpr VulkanUploadTaskType Type = VulkanUploadTaskType::Buffer;
	VulkanBufferUploadTask() { type = Type; }
	VkBuffer buffer;
	VulkanStagingBuffer stagingBuffer;
	VkDeviceSize size;
};

template <typename T>
static T* dyn_cast(VulkanUploadTask* ptr)
{
	if (ptr->type == T::Type)
	{
		return static_cast<T*>(ptr);
	}
	return nullptr;
}

class VulkanUploadQueue
{
public:
	explicit VulkanUploadQueue(VulkanDevice* device, const PrismObj<VulkanQueueStore>& transferQueueStore);
	~VulkanUploadQueue();

	VulkanUploadQueue(const VulkanUploadQueue&) = delete;
	VulkanUploadQueue& operator=(const VulkanUploadQueue&) = delete;

	VulkanStagingBufferPool& GetStagingPool() { return stagingPool; }

	void Enqueue(VulkanUploadTask* task);
	void WaitIdle();

	uint64_t GetPollingRateMax() const { return pollingRateMax; }
	void SetPollingRateMax(uint64_t value) { pollingRateMax = value; }

private:
	struct UploadSubmission
	{
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
	};

	struct InFlightTask
	{
		VulkanUploadTask* task;
		UploadSubmission submission;
	};

	void ThreadMain();
	void SubmitTask(VulkanUploadTask* task);
	void PollInFlight();
	UploadSubmission AcquireSubmission();
	void ReleaseSubmission(UploadSubmission submission);

	VulkanDevice* device;
	VulkanStagingBufferPool stagingPool;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	HEXA_UTILS_NAMESPACE::fmutex pendingMutex;
	std::queue<VulkanUploadTask*> pending;

	std::vector<InFlightTask> inFlight;
	std::vector<UploadSubmission> freeSubmissions;

	// Caps how many idle {command buffer, fence} pairs stick around after a burst - beyond
	// this, ReleaseSubmission() destroys them instead of pooling, so a one-off spike doesn't
	// permanently reserve driver resources sized for its peak.
	static constexpr size_t MaxFreeSubmissions = 64;

	static constexpr uint64_t DefaultPollingRateMax = 1'000'000;
	uint64_t pollingRateMax = DefaultPollingRateMax;

	std::atomic<uint64_t> outstandingCount{ 0 };
	std::atomic<uint64_t> pendingSignal{ 0 };

	std::thread thread;
	std::atomic<bool> running{ true };

	PrismObj<VulkanQueueStore> transferQueueStore;
};

HEXA_PRISM_NAMESPACE_END

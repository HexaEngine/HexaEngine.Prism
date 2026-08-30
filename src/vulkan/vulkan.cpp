#define VMA_IMPLEMENTATION
#include "vulkan/vulkan.hpp"
#include "vulkan/resource_binding_list.hpp"
#include "vulkan/upload_queue.hpp"
#include "vulkan/helper.hpp"
#include <algorithm>
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <SDL3/SDL_vulkan.h>
#include <spirv_reflect.h>

HEXA_PRISM_NAMESPACE_BEGIN

#define VK_CHK(result) \
    if (result != VK_SUCCESS) { return false; }

#define VK_CHKRETDEF(result) \
    if (result != VK_SUCCESS) { return {}; }

namespace
{
    void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspect)
    {
        if (oldLayout == newLayout || image == VK_NULL_HANDLE)
        {
            return;
        }

        VkImageMemoryBarrier2 barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = { aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };

        VkDependencyInfo depInfo = {};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    void TransitionResourceTo(VkCommandBuffer cmd, Resource* resource, VkImageLayout newLayout, VkImageAspectFlags aspect)
    {
        if (!resource)
        {
            return;
        }
        VkImage image = GetVkImage(resource);
        VkImageLayout oldLayout = GetTrackedLayout(resource);
        TransitionImage(cmd, image, oldLayout, newLayout, aspect);
        SetTrackedLayout(resource, newLayout);
    }

    void BindDescriptorSets(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, ResourceBindingList& bindings)
    {
        auto& vkBindings = static_cast<VulkanResourceBindingList&>(bindings);
        const auto& sets = vkBindings.GetDescriptorSets();
        if (!sets.empty())
        {
            vkCmdBindDescriptorSets(cmd, bindPoint, layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
        }
    }

    bool UploadBufferData(VulkanDevice* device, VkBuffer dstBuffer, const void* data, size_t size)
    {
        VulkanUploadQueue* uploadQueue = device->GetUploadQueue();

        VulkanStagingBuffer staging = uploadQueue->GetStagingPool().Rent(size);
        PrismMemoryCopy(staging.mappedData, data, size);

        VulkanBufferUploadTask task;
        task.buffer = dstBuffer;
        task.stagingBuffer = staging;
        task.size = size;

        WaitFlag uploadDone;
        task.completionFlag = &uploadDone;

        uploadQueue->Enqueue(&task);
        uploadDone.Wait();
        return true;
    }

    void UploadInitialTextureData(VulkanDevice* device, VkImage image, uint32_t width, uint32_t height, uint32_t depth,
        uint32_t mipLevels, uint32_t arraySize, bool is3D, const SubresourceData* initialData)
    {
        if (!initialData)
        {
            return;
        }

        uint32_t subresourceCount = is3D ? mipLevels : arraySize * mipLevels;

        VulkanUploadQueue* uploadQueue = device->GetUploadQueue();

        VulkanImageUploadTask task;
        task.image = image;
        task.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        task.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        task.stagingBuffers.reserve(subresourceCount);
        task.regions.reserve(subresourceCount);

        for (uint32_t i = 0; i < subresourceCount; ++i)
        {
            uint32_t mip = is3D ? i : i % mipLevels;
            uint32_t arraySlice = is3D ? 0 : i / mipLevels;
            uint32_t mipWidth = std::max(1u, width >> mip);
            uint32_t mipHeight = std::max(1u, height >> mip);
            uint32_t mipDepth = is3D ? std::max(1u, depth >> mip) : 1;
            size_t size = static_cast<size_t>(initialData[i].slicePitch) * mipDepth;

            VulkanStagingBuffer staging = uploadQueue->GetStagingPool().Rent(size);
            PrismMemoryCopy(staging.mappedData, initialData[i].data, size);

            VulkanImageUploadRegion region = {};
            region.mipLevel = mip;
            region.baseArrayLayer = arraySlice;
            region.width = mipWidth;
            region.height = mipHeight;
            region.depth = mipDepth;
            region.bufferOffset = 0;

            task.stagingBuffers.push_back(staging);
            task.regions.push_back(region);
        }

        WaitFlag uploadDone;
        task.completionFlag = &uploadDone;

        uploadQueue->Enqueue(&task);
        uploadDone.Wait();
    }
}

static int32_t ComputeQueueFamilyScore(const VkQueueFamilyProperties& fam, VkQueueFlags penaltyFlags)
{
    int32_t penalty = std::popcount(fam.queueFlags & (penaltyFlags));
    return static_cast<int32_t>(fam.queueCount) - penalty * 1000;
}

static void ApplyFamily(uint32_t index, const VkQueueFamilyProperties& fam, QueueFamilyIndices::QueueFamilyIndex& famIndex, VkQueueFlags penaltyFlags)
{
    auto score = ComputeQueueFamilyScore(fam, penaltyFlags);
    if (famIndex.score < score)
    {
        famIndex = { index, fam.queueCount, fam.queueFlags, score };
    }
}

bool VulkanDevice::FindQueueFamily(QueueFamilyIndices& indices) const
{
    static constexpr auto InvalidIndex = QueueFamilyIndices::InvalidIndex;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, families.data());

    for (uint32_t i = 0; i < count; i++)
    {
        auto& fam = families[i];
        auto flags = fam.queueFlags;
        if ((flags & VK_QUEUE_GRAPHICS_BIT))
        {
            ApplyFamily(i, fam, indices.graphics, VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
        }

        if ((flags & VK_QUEUE_COMPUTE_BIT))
        {
            ApplyFamily(i, fam, indices.compute, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
        }

        if ((flags & VK_QUEUE_TRANSFER_BIT))
        {
            ApplyFamily(i, fam, indices.transfer, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
        }
    }

    if (indices.compute.IsInvalid())
    {
        indices.compute = indices.graphics;
    }

    return indices.graphics.IsValid();
}

bool VulkanDevice::CreateLogicalDevice()
{
    if (!FindQueueFamily(queueIndicies))
        return false;

    std::unordered_set<uint32_t> uniqueFamilies = { queueIndicies.graphics.index };
    if (queueIndicies.compute.IsValid())
        uniqueFamilies.insert(queueIndicies.compute.index);
    if (queueIndicies.transfer.IsValid())
        uniqueFamilies.insert(queueIndicies.transfer.index);

    bool secondGraphicsQueue = queueIndicies.transfer.IsInvalid() && queueIndicies.graphics.queueCount > 1;
    uint32_t graphicsQueueCount = secondGraphicsQueue ? 2 : 1;

    std::vector<float> priorities(graphicsQueueCount, 1.0f);

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (uint32_t family : uniqueFamilies)
    {
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family;
        info.queueCount = family == queueIndicies.graphics.index ? graphicsQueueCount : 1;
        info.pQueuePriorities = priorities.data();
        queueInfos.push_back(info);
    }

    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    VkPhysicalDeviceFeatures features = {};
    features.samplerAnisotropy = VK_TRUE;
    features.drawIndirectFirstInstance = VK_TRUE;
    features.multiDrawIndirect = VK_TRUE;
    features.fullDrawIndexUint32 = VK_TRUE;

    features.fillModeNonSolid = VK_TRUE;
    features.wideLines = VK_TRUE;

    features.depthBiasClamp = VK_TRUE;
    features.depthBounds = VK_TRUE;
    features.depthClamp = VK_TRUE;

    features.independentBlend = VK_TRUE;
    features.alphaToOne = VK_TRUE;
    features.logicOp = VK_TRUE;
    features.dualSrcBlend = VK_TRUE;

    features.occlusionQueryPrecise = VK_TRUE;
    features.pipelineStatisticsQuery = VK_TRUE;
    features.shaderClipDistance = VK_TRUE;
    features.shaderCullDistance = VK_TRUE;
    features.shaderInt64 = VK_TRUE;
    features.shaderInt16 = VK_TRUE;
    features.shaderFloat64 = VK_TRUE;

    features.geometryShader = VK_TRUE;
    features.tessellationShader = VK_TRUE;
    features.textureCompressionBC = VK_TRUE;

    features.shaderStorageImageMultisample = VK_TRUE;
    features.sampleRateShading = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.descriptorIndexing = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE; 
    features12.timelineSemaphore = VK_TRUE; 
    features12.shaderFloat16 = VK_TRUE;
    features12.shaderOutputViewportIndex = VK_TRUE;

    VkPhysicalDeviceVulkan13Features features13 = {};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    VkDeviceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
        .pQueueCreateInfos = queueInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &features,
    };

    features13.pNext = &features12;
    createInfo.pNext = &features13;

    VK_CHK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

    graphicsQueue = MakePrismObj<VulkanQueueStore>(device, queueIndicies.graphics.index, 0);
    if (queueIndicies.compute.IsValid())
    {
        computeQueue = MakePrismObj<VulkanQueueStore>(device, queueIndicies.compute.index, 0);
    }
    if (queueIndicies.transfer.IsValid())
    {
        transferQueue = MakePrismObj<VulkanQueueStore>(device, queueIndicies.transfer.index, 0);
    }
    else
    {
        if (queueIndicies.graphics.queueCount > 1)
        {
            transferQueue = MakePrismObj<VulkanQueueStore>(device, queueIndicies.graphics.index, 1);
        }
        else
        {
            transferQueue = graphicsQueue;
        }
    }

    return true;
}

namespace
{
    VkBool32 VKAPI_PTR DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void* userData)
    {
        DebugMessageSeverity mapped;
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) mapped = DebugMessageSeverity::Error;
        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) mapped = DebugMessageSeverity::Warning;
        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) mapped = DebugMessageSeverity::Info;
        else mapped = DebugMessageSeverity::Verbose;

        auto* device = static_cast<VulkanDevice*>(userData);
        if (device && device->GetDebugCallback())
        {
            device->GetDebugCallback()(mapped, data->pMessage, device->GetDebugCallbackUserData());
        }
        else
        {
            std::cout << data->pMessage << std::endl;
        }
        return VK_FALSE;
    }
}

bool VulkanDevice::Initialize(const DeviceDesc& desc)
{
    std::vector<const char*> layers;
    if ((desc.flags & DeviceFlags::Debug) != DeviceFlags::None)
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (auto& l : availableLayers)
        {
            if (strcmp(l.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            {
                layers.push_back("VK_LAYER_KHRONOS_validation");
            }
        }
    }

    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if HEXA_PRISM_WINDOWS
    if ((desc.flags & DeviceFlags::Win32) != 0)
    {
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#endif

#if HEXA_PRISM_LINUX
    if ((desc.flags & GraphicsDeviceFlags::X11) != 0)
    {
        extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    }
    if ((desc.flags & GraphicsDeviceFlags::Wayland) != 0)
    {
        extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    }
#endif

    bool debugMessengerEnabled = (desc.flags & DeviceFlags::Debug) != DeviceFlags::None && !layers.empty();
    if (debugMessengerEnabled)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        debugCallback = desc.debugCallback;
        debugCallbackUserData = desc.debugCallbackUserData;
    }

    VkApplicationInfo appInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "HexaEngine Prism",
        .apiVersion = VK_API_VERSION_1_3
    };

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
    messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerInfo.pfnUserCallback = DebugMessengerCallback;
    messengerInfo.pUserData = this;

    VkInstanceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = debugMessengerEnabled ? &messengerInfo : nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VkResult result;

    VK_CHK(vkCreateInstance(&createInfo, NULL, &instance));

    if (debugMessengerEnabled)
    {
        auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (createMessenger)
        {
            createMessenger(instance, &messengerInfo, nullptr, &debugMessenger);
        }
    }

    uint32_t deviceCount = 0;
    VK_CHK(vkEnumeratePhysicalDevices(instance, &deviceCount, NULL));
    if (deviceCount == 0)
    {
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

    VkPhysicalDevice phy = VK_NULL_HANDLE;
    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            phy = device;
            break;
        }
    }

    if (phy == VK_NULL_HANDLE)
    {
        phy = devices[0];
    }

    physicalDevice = phy;

    if (!CreateLogicalDevice())
    {
        return false;
    }

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
    {
        return false;
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueIndicies.graphics.index;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHK(vkCreateCommandPool(device, &poolInfo, GetAllocationCallbacks(), &commandPool));

    constexpr uint32_t MaxDescriptorSets = 4096;
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MaxDescriptorSets },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxDescriptorSets },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MaxDescriptorSets },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxDescriptorSets },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MaxDescriptorSets },
        { VK_DESCRIPTOR_TYPE_SAMPLER, MaxDescriptorSets },
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolInfo.maxSets = MaxDescriptorSets;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    descriptorPoolInfo.pPoolSizes = poolSizes;
    VK_CHK(vkCreateDescriptorPool(device, &descriptorPoolInfo, GetAllocationCallbacks(), &descriptorPool));

    uploadQueue = make_uptr<VulkanUploadQueue>(this, transferQueue);

    if (!CreateClearUAVPipeline())
    {
        return false;
    }

    return true;
}

bool VulkanDevice::CreateClearUAVPipeline()
{
    auto shaderSource = MakePrismObj<TextShaderSource>("ClearUAV", R"(
    RWTexture2D<float4> Target : register(u0);

    struct ClearParams
    {
        int2 offset;
        uint2 extent;
        float4 color;
    };
    [[vk::push_constant]] ClearParams params;

    [numthreads(8, 8, 1)]
    void main(uint3 dispatchID : SV_DispatchThreadID)
    {
        if (dispatchID.x >= params.extent.x || dispatchID.y >= params.extent.y)
        {
            return;
        }
        Target[params.offset + int2(dispatchID.xy)] = params.color;
    }
    )");

    PrismObj<Blob> spirv;
    if (!VulkanShaderCompiler::Compile(shaderSource.Get(), "main", ShaderStage::Compute, spirv))
    {
        return false;
    }

    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = spirv->GetLength();
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(spirv->GetData());
    if (vkCreateShaderModule(device, &moduleInfo, GetAllocationCallbacks(), &clearUAVShaderModule) != VK_SUCCESS)
    {
        return false;
    }

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, GetAllocationCallbacks(), &clearUAVDescriptorSetLayout) != VK_SUCCESS)
    {
        return false;
    }

    VkPushConstantRange pushRange = {};
    pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(float) * 8;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &clearUAVDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, GetAllocationCallbacks(), &clearUAVPipelineLayout) != VK_SUCCESS)
    {
        return false;
    }

    VkPipelineShaderStageCreateInfo stageInfo = {};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = clearUAVShaderModule;
    stageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = clearUAVPipelineLayout;
    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, GetAllocationCallbacks(), &clearUAVPipeline) != VK_SUCCESS)
    {
        return false;
    }

    VkDescriptorSetLayout setLayouts[ClearUAVDescriptorSetCount];
    for (auto& layout : setLayouts)
    {
        layout = clearUAVDescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = ClearUAVDescriptorSetCount;
    allocInfo.pSetLayouts = setLayouts;
    return vkAllocateDescriptorSets(device, &allocInfo, clearUAVDescriptorSets) == VK_SUCCESS;
}

VulkanDevice::VulkanDevice() = default;

VulkanDevice::~VulkanDevice()
{
    uploadQueue.reset();

    if (clearUAVPipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, clearUAVPipeline, GetAllocationCallbacks());
    if (clearUAVPipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, clearUAVPipelineLayout, GetAllocationCallbacks());
    if (clearUAVDescriptorSetLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(device, clearUAVDescriptorSetLayout, GetAllocationCallbacks());
    if (clearUAVShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device, clearUAVShaderModule, GetAllocationCallbacks());

    if (debugMessenger != VK_NULL_HANDLE)
    {
        auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroyMessenger)
        {
            destroyMessenger(instance, debugMessenger, nullptr);
        }
    }
}

CommandQueue* VulkanDevice::GetCommandQueue(uint32_t index)
{
    if (index != 0)
    {
        return nullptr; // only the graphics queue is exposed for now
    }

    if (!graphicsCommandQueue)
    {
        CommandQueueDesc desc = {};
        desc.type = CommandQueueType::Graphics;
        desc.index = 0;
        desc.priority = 1.0f;
        graphicsCommandQueue = MakePrismObj<VulkanCommandQueue>(desc, this, graphicsQueue);
    }

    return graphicsCommandQueue.Get();
}

PrismObj<Fence> VulkanDevice::CreateFence(uint64_t initialValue)
{
    VkSemaphoreTypeCreateInfo typeInfo = {};
    typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeInfo.initialValue = initialValue;

    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &typeInfo;

    VkSemaphore semaphore;
    VK_CHKRETDEF(vkCreateSemaphore(device, &createInfo, GetAllocationCallbacks(), &semaphore));

    return MakePrismObj<VulkanFence>(this, semaphore);
}

PrismObj<CommandAllocator> VulkanDevice::CreateCommandAllocator(const CommandAllocatorDesc& desc)
{
    auto queue = static_cast<VulkanCommandQueue*>(desc.queue);
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueIndicies.graphics.index;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPool pool;
    VK_CHKRETDEF(vkCreateCommandPool(device, &poolInfo, GetAllocationCallbacks(), &pool));

    return MakePrismObj<VulkanCommandAllocator>(desc, this, pool);
}

PrismObj<CommandList> VulkanDevice::CreateCommandList(const CommandListDesc& desc)
{
    VkCommandPool pool = commandPool;
    if (auto* vkAllocator = dynamic_cast<VulkanCommandAllocator*>(desc.allocator))
    {
        pool = vkAllocator->GetCommandPool();
    }

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHKRETDEF(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    return MakePrismObj<VulkanCommandList>(desc, this, commandBuffer);
}

PrismObj<Buffer> VulkanDevice::CreateBuffer(const BufferDesc& desc, const SubresourceData* initialData)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.widthInBytes;
    bufferInfo.usage = ConvertBufferUsageFlags(desc);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = ConvertAllocationInfo(desc.cpuAccessFlags, desc.gpuAccessFlags);

    VkBuffer buffer;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo allocInfoOut;
    if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &bufferAllocation, &allocInfoOut) != VK_SUCCESS)
    {
        return {};
    }

    if (initialData && initialData->data && desc.widthInBytes > 0)
    {
        if (allocInfoOut.pMappedData)
        {
            PrismMemoryCopy(allocInfoOut.pMappedData, initialData->data, desc.widthInBytes);
        }
        else
        {
            UploadBufferData(this, buffer, initialData->data, desc.widthInBytes);
        }
    }

    return MakePrismObj<VulkanBuffer>(this, desc, buffer, bufferAllocation);
}

PrismObj<Texture1D> VulkanDevice::CreateTexture1D(const Texture1DDesc& desc, const SubresourceData* initialData)
{
    VkFormat format = ConvertFormat(desc.format);
    uint32_t mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;
    uint32_t arraySize = desc.arraySize == 0 ? 1 : desc.arraySize;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_1D;
    imageInfo.format = format;
    imageInfo.extent = { desc.width, 1, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arraySize;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = ConvertImageUsageFlags(desc.gpuAccessFlags, format);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocCreateInfo = ConvertAllocationInfo(desc.cpuAccessFlags, desc.gpuAccessFlags);

    VkImage image;
    VmaAllocation imageAllocation;
    if (vmaCreateImage(allocator, &imageInfo, &allocCreateInfo, &image, &imageAllocation, nullptr) != VK_SUCCESS)
    {
        return {};
    }

    UploadInitialTextureData(this, image, desc.width, 1, 1, mipLevels, arraySize, false, initialData);

    auto tex = MakePrismObj<VulkanTexture1D>(this, desc, image, imageAllocation);
    if (initialData)
    {
        tex->SetCurrentLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return tex;
}

PrismObj<Texture2D> VulkanDevice::CreateTexture2D(const Texture2DDesc& desc, const SubresourceData* initialData)
{
    VkFormat format = ConvertFormat(desc.format);
    uint32_t mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;
    uint32_t arraySize = desc.arraySize == 0 ? 1 : desc.arraySize;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { desc.width, desc.height, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arraySize;
    imageInfo.samples = ConvertSampleCount(desc.sampleDesc.count);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = ConvertImageUsageFlags(desc.gpuAccessFlags, format);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if ((desc.miscFlags & ResourceMiscFlags::TextureCube) != ResourceMiscFlags::None)
    {
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VmaAllocationCreateInfo allocCreateInfo = ConvertAllocationInfo(desc.cpuAccessFlags, desc.gpuAccessFlags);

    VkImage image;
    VmaAllocation imageAllocation;
    if (vmaCreateImage(allocator, &imageInfo, &allocCreateInfo, &image, &imageAllocation, nullptr) != VK_SUCCESS)
    {
        return {};
    }

    UploadInitialTextureData(this, image, desc.width, desc.height, 1, mipLevels, arraySize, false, initialData);

    auto tex = MakePrismObj<VulkanTexture2D>(this, desc, image, imageAllocation);
    if (initialData)
    {
        tex->SetCurrentLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return tex;
}

PrismObj<Texture3D> VulkanDevice::CreateTexture3D(const Texture3DDesc& desc, const SubresourceData* initialData)
{
    VkFormat format = ConvertFormat(desc.format);
    uint32_t mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.format = format;
    imageInfo.extent = { desc.width, desc.height, desc.depth };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = ConvertImageUsageFlags(desc.gpuAccessFlags, format);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocCreateInfo = ConvertAllocationInfo(desc.cpuAccessFlags, desc.gpuAccessFlags);

    VkImage image;
    VmaAllocation imageAllocation;
    if (vmaCreateImage(allocator, &imageInfo, &allocCreateInfo, &image, &imageAllocation, nullptr) != VK_SUCCESS)
    {
        return {};
    }

    UploadInitialTextureData(this, image, desc.width, desc.height, desc.depth, mipLevels, 1, true, initialData);

    auto tex = MakePrismObj<VulkanTexture3D>(this, desc, image, imageAllocation);
    if (initialData)
    {
        tex->SetCurrentLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return tex;
}

PrismObj<RenderTargetView> VulkanDevice::CreateRenderTargetView(Resource* resource, const RenderTargetViewDesc& desc)
{
    if (!resource)
    {
        return {};
    }

    VkImage image = GetVkImage(resource);
    if (image == VK_NULL_HANDLE)
    {
        return {};
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.format = ConvertFormat(ResolveViewFormat(resource, desc.format));
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    switch (desc.dimension)
    {
    case RenderTargetViewDimension::Texture1D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1D.mipSlice;
        break;
    case RenderTargetViewDimension::Texture1DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1DArray.mipSlice;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture1DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture1DArray.arraySize;
        break;
    case RenderTargetViewDimension::Texture2D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2D.mipSlice;
        break;
    case RenderTargetViewDimension::Texture2DMS:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case RenderTargetViewDimension::Texture2DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2DArray.mipSlice;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DArray.arraySize;
        break;
    case RenderTargetViewDimension::Texture2DMSArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DMSArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DMSArray.arraySize;
        break;
    case RenderTargetViewDimension::Texture3D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture3D.mipSlice;
        break;
    default:
        return {};
    }

    VkImageView view;
    if (vkCreateImageView(device, &viewInfo, GetAllocationCallbacks(), &view) != VK_SUCCESS)
    {
        return {};
    }

    return MakePrismObj<VulkanRenderTargetView>(this, view, resource, desc);
}

PrismObj<ShaderResourceView> VulkanDevice::CreateShaderResourceView(Resource* resource, const ShaderResourceViewDesc& desc)
{
    if (!resource)
    {
        return {};
    }

    VkImage image = GetVkImage(resource);
    if (image == VK_NULL_HANDLE)
    {
        return {}; // buffer SRVs not yet supported
    }

    VkFormat format = ConvertFormat(ResolveViewFormat(resource, desc.format));

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = ConvertAspectFlags(format);
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    switch (desc.dimension)
    {
    case ShaderResourceViewDimension::Texture1D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1D.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.texture1D.mipLevels;
        break;
    case ShaderResourceViewDimension::Texture1DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1DArray.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.texture1DArray.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture1DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture1DArray.arraySize;
        break;
    case ShaderResourceViewDimension::Texture2D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2D.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.texture2D.mipLevels;
        break;
    case ShaderResourceViewDimension::Texture2DMS:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case ShaderResourceViewDimension::Texture2DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2DArray.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.texture2DArray.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DArray.arraySize;
        break;
    case ShaderResourceViewDimension::Texture2DMSArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DMSArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DMSArray.arraySize;
        break;
    case ShaderResourceViewDimension::Texture3D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture3D.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.texture3D.mipLevels;
        break;
    case ShaderResourceViewDimension::TextureCube:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.subresourceRange.baseMipLevel = desc.textureCube.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.textureCube.mipLevels;
        viewInfo.subresourceRange.layerCount = 6;
        break;
    case ShaderResourceViewDimension::TextureCubeArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.textureCubeArray.mostDetailedMip;
        viewInfo.subresourceRange.levelCount = desc.textureCubeArray.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = desc.textureCubeArray.first2DArrayFace;
        viewInfo.subresourceRange.layerCount = desc.textureCubeArray.numCubes * 6;
        break;
    default:
        return {};
    }

    VkImageView view;
    if (vkCreateImageView(device, &viewInfo, GetAllocationCallbacks(), &view) != VK_SUCCESS)
    {
        return {};
    }

    return MakePrismObj<VulkanShaderResourceView>(this, view, resource, desc);
}

PrismObj<DepthStencilView> VulkanDevice::CreateDepthStencilView(Resource* resource, const DepthStencilViewDesc& desc)
{
    if (!resource)
    {
        return {};
    }

    VkImage image = GetVkImage(resource);
    if (image == VK_NULL_HANDLE)
    {
        return {};
    }

    VkFormat format = ConvertFormat(ResolveViewFormat(resource, desc.format));

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = ConvertAspectFlags(format);
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    switch (desc.dimension)
    {
    case DepthStencilViewDimension::Texture1D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1D.mipSlice;
        break;
    case DepthStencilViewDimension::Texture1DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1DArray.mipSlice;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture1DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture1DArray.arraySize;
        break;
    case DepthStencilViewDimension::Texture2D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2D.mipSlice;
        break;
    case DepthStencilViewDimension::Texture2DMS:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case DepthStencilViewDimension::Texture2DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2DArray.mipSlice;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DArray.arraySize;
        break;
    case DepthStencilViewDimension::Texture2DMSArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DMSArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DMSArray.arraySize;
        break;
    default:
        return {};
    }

    VkImageView view;
    if (vkCreateImageView(device, &viewInfo, GetAllocationCallbacks(), &view) != VK_SUCCESS)
    {
        return {};
    }

    return MakePrismObj<VulkanDepthStencilView>(this, view, resource, desc);
}

PrismObj<UnorderedAccessView> VulkanDevice::CreateUnorderedAccessView(Resource* resource, const UnorderedAccessViewDesc& desc)
{
    if (!resource)
    {
        return {};
    }

    VkImage image = GetVkImage(resource);
    if (image == VK_NULL_HANDLE)
    {
        return {}; // buffer UAVs not yet supported
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.format = ConvertFormat(ResolveViewFormat(resource, desc.format));
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    switch (desc.dimension)
    {
    case UnorderedAccessViewDimension::Texture1D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1D.mipSlice;
        break;
    case UnorderedAccessViewDimension::Texture1DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture1DArray.mipSlice;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture1DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture1DArray.arraySize;
        break;
    case UnorderedAccessViewDimension::Texture2D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2D.mipSlice;
        break;
    case UnorderedAccessViewDimension::Texture2DArray:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.subresourceRange.baseMipLevel = desc.texture2DArray.mipSlice;
        viewInfo.subresourceRange.baseArrayLayer = desc.texture2DArray.firstArraySlice;
        viewInfo.subresourceRange.layerCount = desc.texture2DArray.arraySize;
        break;
    case UnorderedAccessViewDimension::Texture3D:
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.subresourceRange.baseMipLevel = desc.texture3D.mipSlice;
        break;
    default:
        return {};
    }

    VkImageView view;
    if (vkCreateImageView(device, &viewInfo, GetAllocationCallbacks(), &view) != VK_SUCCESS)
    {
        return {};
    }

    return MakePrismObj<VulkanUnorderedAccessView>(this, view, resource, desc);
}

PrismObj<SamplerState> VulkanDevice::CreateSamplerState(const SamplerDesc& desc)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = ConvertMinMagFilter(desc.filter);
    samplerInfo.minFilter = samplerInfo.magFilter;
    samplerInfo.mipmapMode = ConvertMipmapMode(desc.filter);
    samplerInfo.addressModeU = ConvertAddressMode(desc.addressU);
    samplerInfo.addressModeV = ConvertAddressMode(desc.addressV);
    samplerInfo.addressModeW = ConvertAddressMode(desc.addressW);
    samplerInfo.mipLodBias = desc.mipLODBias;
    samplerInfo.anisotropyEnable = IsAnisotropicFilter(desc.filter) ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = static_cast<float>(desc.maxAnisotropy);
    samplerInfo.compareEnable = IsComparisonFilter(desc.filter) ? VK_TRUE : VK_FALSE;
    samplerInfo.compareOp = ConvertCompareOp(desc.comparisonFunc);
    samplerInfo.minLod = desc.minLOD;
    samplerInfo.maxLod = desc.maxLOD;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler;
    if (vkCreateSampler(device, &samplerInfo, GetAllocationCallbacks(), &sampler) != VK_SUCCESS)
    {
        return {};
    }

    return MakePrismObj<VulkanSamplerState>(this, sampler, desc);
}

namespace
{
    bool CompileStage(const PrismObj<ShaderSource>& source, const char* entryPoint, ShaderStage stage, PrismObj<Blob>& spirvOut)
    {
        if (!source)
        {
            return true; // optional stage
        }
        return VulkanShaderCompiler::Compile(source.Get(), entryPoint, stage, spirvOut);
    }

    bool CreateModuleFromSpirv(VulkanDevice* device, const PrismObj<Blob>& spirv, VkShaderModule& moduleOut)
    {
        if (!spirv)
        {
            return true; // optional stage that was never compiled
        }

        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = spirv->GetLength();
        moduleInfo.pCode = reinterpret_cast<const uint32_t*>(spirv->GetData());

        return vkCreateShaderModule(device->GetDevice(), &moduleInfo, device->GetAllocationCallbacks(), &moduleOut) == VK_SUCCESS;
    }

    bool CompileAndCreateModule(VulkanDevice* device, const PrismObj<ShaderSource>& source, const char* entryPoint, ShaderStage stage, VkShaderModule& moduleOut, PrismObj<Blob>& spirvOut)
    {
        return CompileStage(source, entryPoint, stage, spirvOut) && CreateModuleFromSpirv(device, spirvOut, moduleOut);
    }

    void AssignStageDescriptorSet(PrismObj<Blob>& spirv, uint32_t setIndex)
    {
        if (!spirv || setIndex == 0)
        {
            return;
        }

        SpvReflectShaderModule module = {};
        if (spvReflectCreateShaderModule(spirv->GetLength(), spirv->GetData(), &module) != SPV_REFLECT_RESULT_SUCCESS)
        {
            return;
        }

        uint32_t count = 0;
        spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
        std::vector<SpvReflectDescriptorBinding*> vars(count);
        spvReflectEnumerateDescriptorBindings(&module, &count, vars.data());

        for (auto* var : vars)
        {
            spvReflectChangeDescriptorBindingNumbers(&module, var, var->binding, setIndex);
        }

        if (!vars.empty())
        {
            size_t codeSize = spvReflectGetCodeSize(&module);
            const uint32_t* code = spvReflectGetCode(&module);
            uint8_t* rewritten = PrismAllocT<uint8_t>(codeSize);
            PrismMemoryCopy(rewritten, code, codeSize);
            spirv = MakePrismObj<Blob>(rewritten, codeSize, true);
        }

        spvReflectDestroyShaderModule(&module);
    }

    VkPipelineShaderStageCreateInfo MakeStageInfo(VkShaderStageFlagBits stage, VkShaderModule module, const char* entryPoint)
    {
        VkPipelineShaderStageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = stage;
        info.module = module;
        info.pName = entryPoint && *entryPoint ? entryPoint : "main";
        return info;
    }

    // Mirrors D3D11's auto-derivation of the vertex input layout from shader reflection
    // (GetInputElementsFromSignature), used when the caller doesn't supply explicit
    // InputElementDescriptions. Assumes a single interleaved vertex buffer at binding 0.
    void ReflectVertexInputState(const PrismObj<Blob>& vertexSpirv, std::vector<VkVertexInputBindingDescription>& bindings, std::vector<VkVertexInputAttributeDescription>& attributes)
    {
        if (!vertexSpirv)
        {
            return;
        }

        SpvReflectShaderModule module = {};
        if (spvReflectCreateShaderModule(vertexSpirv->GetLength(), vertexSpirv->GetData(), &module) != SPV_REFLECT_RESULT_SUCCESS)
        {
            return;
        }

        uint32_t count = 0;
        spvReflectEnumerateInputVariables(&module, &count, nullptr);
        std::vector<SpvReflectInterfaceVariable*> vars(count);
        spvReflectEnumerateInputVariables(&module, &count, vars.data());

        std::sort(vars.begin(), vars.end(), [](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) { return a->location < b->location; });

        uint32_t offset = 0;
        for (auto* var : vars)
        {
            if (var->built_in != -1)
            {
                continue; // e.g. SV_VertexID/SV_InstanceID: not backed by a vertex buffer attribute
            }

            VkVertexInputAttributeDescription attr = {};
            attr.location = var->location;
            attr.binding = 0;
            attr.format = static_cast<VkFormat>(var->format);
            attr.offset = offset;
            attributes.push_back(attr);

            uint32_t componentCount = var->numeric.vector.component_count > 0 ? var->numeric.vector.component_count : 1;
            uint32_t componentWidth = var->numeric.scalar.width > 0 ? var->numeric.scalar.width / 8 : 4;
            offset += componentCount * componentWidth;
        }

        spvReflectDestroyShaderModule(&module);

        if (!attributes.empty())
        {
            VkVertexInputBindingDescription binding = {};
            binding.binding = 0;
            binding.stride = offset;
            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindings.push_back(binding);
        }
    }
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, const GraphicsPipelineDesc& desc)
    : VulkanDeviceChild(device), GraphicsPipeline(desc)
{
    Compile();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    VkDevice vkDevice = device->GetDevice();
    const VkAllocationCallbacks* alloc = device->GetAllocationCallbacks();
    if (vertexModule) vkDestroyShaderModule(vkDevice, vertexModule, alloc);
    if (hullModule) vkDestroyShaderModule(vkDevice, hullModule, alloc);
    if (domainModule) vkDestroyShaderModule(vkDevice, domainModule, alloc);
    if (geometryModule) vkDestroyShaderModule(vkDevice, geometryModule, alloc);
    if (pixelModule) vkDestroyShaderModule(vkDevice, pixelModule, alloc);
}

void VulkanGraphicsPipeline::Compile()
{
    bool success = true;
    success &= CompileStage(desc.vertexShader, desc.vertexEntryPoint, ShaderStage::Vertex, vertexSpirv);
    success &= CompileStage(desc.hullShader, desc.hullEntryPoint, ShaderStage::Hull, hullSpirv);
    success &= CompileStage(desc.domainShader, desc.domainEntryPoint, ShaderStage::Domain, domainSpirv);
    success &= CompileStage(desc.geometryShader, desc.geometryEntryPoint, ShaderStage::Geometry, geometrySpirv);
    success &= CompileStage(desc.pixelShader, desc.pixelEntryPoint, ShaderStage::Pixel, pixelSpirv);

    AssignStageDescriptorSet(vertexSpirv, 0);
    AssignStageDescriptorSet(hullSpirv, 1);
    AssignStageDescriptorSet(domainSpirv, 2);
    AssignStageDescriptorSet(geometrySpirv, 3);
    AssignStageDescriptorSet(pixelSpirv, 4);

    success &= CreateModuleFromSpirv(device, vertexSpirv, vertexModule);
    success &= CreateModuleFromSpirv(device, hullSpirv, hullModule);
    success &= CreateModuleFromSpirv(device, domainSpirv, domainModule);
    success &= CreateModuleFromSpirv(device, geometrySpirv, geometryModule);
    success &= CreateModuleFromSpirv(device, pixelSpirv, pixelModule);

    valid = success;
}

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device, const ComputePipelineDesc& desc)
    : VulkanDeviceChild(device), ComputePipeline(desc)
{
    Compile();
}

VulkanComputePipeline::~VulkanComputePipeline()
{
    if (computeModule)
    {
        vkDestroyShaderModule(device->GetDevice(), computeModule, device->GetAllocationCallbacks());
    }
}

void VulkanComputePipeline::Compile()
{
    valid = CompileAndCreateModule(device, desc.computeShader, desc.computeEntryPoint, ShaderStage::Compute, computeModule, computeSpirv);
}

VulkanGraphicsPipelineState::VulkanGraphicsPipelineState(VulkanDevice* device, const PrismObj<GraphicsPipeline>& pipeline, const GraphicsPipelineStateDesc& desc)
    : VulkanDeviceChild(device), GraphicsPipelineState(pipeline, desc)
{
    Create();
}

VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState()
{
    VkDevice vkDevice = device->GetDevice();
    const VkAllocationCallbacks* alloc = device->GetAllocationCallbacks();
    if (vkPipeline) vkDestroyPipeline(vkDevice, vkPipeline, alloc);
    if (layout) vkDestroyPipelineLayout(vkDevice, layout, alloc);
}

ResourceBindingList& VulkanGraphicsPipelineState::GetBindings() { return *bindingList; }

void VulkanGraphicsPipelineState::Create()
{
    auto* vkGraphicsPipeline = static_cast<VulkanGraphicsPipeline*>(pipeline.Get());
    if (!vkGraphicsPipeline || !vkGraphicsPipeline->IsValid())
    {
        return;
    }

    std::vector<VulkanReflectedStage> reflectedStages;
    if (vkGraphicsPipeline->GetVertexSpirv()) reflectedStages.push_back({ reinterpret_cast<const uint32_t*>(vkGraphicsPipeline->GetVertexSpirv()->GetData()), vkGraphicsPipeline->GetVertexSpirv()->GetLength(), VK_SHADER_STAGE_VERTEX_BIT });
    if (vkGraphicsPipeline->GetHullSpirv()) reflectedStages.push_back({ reinterpret_cast<const uint32_t*>(vkGraphicsPipeline->GetHullSpirv()->GetData()), vkGraphicsPipeline->GetHullSpirv()->GetLength(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT });
    if (vkGraphicsPipeline->GetDomainSpirv()) reflectedStages.push_back({ reinterpret_cast<const uint32_t*>(vkGraphicsPipeline->GetDomainSpirv()->GetData()), vkGraphicsPipeline->GetDomainSpirv()->GetLength(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT });
    if (vkGraphicsPipeline->GetGeometrySpirv()) reflectedStages.push_back({ reinterpret_cast<const uint32_t*>(vkGraphicsPipeline->GetGeometrySpirv()->GetData()), vkGraphicsPipeline->GetGeometrySpirv()->GetLength(), VK_SHADER_STAGE_GEOMETRY_BIT });
    if (vkGraphicsPipeline->GetPixelSpirv()) reflectedStages.push_back({ reinterpret_cast<const uint32_t*>(vkGraphicsPipeline->GetPixelSpirv()->GetData()), vkGraphicsPipeline->GetPixelSpirv()->GetLength(), VK_SHADER_STAGE_FRAGMENT_BIT });

    bindingList = std::make_unique<VulkanResourceBindingList>(device, pipeline.Get(), reflectedStages);

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast<uint32_t>(bindingList->GetSetLayouts().size());
    layoutInfo.pSetLayouts = bindingList->GetSetLayouts().data();
    if (vkCreatePipelineLayout(device->GetDevice(), &layoutInfo, device->GetAllocationCallbacks(), &layout) != VK_SUCCESS)
    {
        return;
    }

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    if (vkGraphicsPipeline->GetVertexModule()) stages.push_back(MakeStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vkGraphicsPipeline->GetVertexModule(), vkGraphicsPipeline->GetDesc().vertexEntryPoint));
    if (vkGraphicsPipeline->GetHullModule()) stages.push_back(MakeStageInfo(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, vkGraphicsPipeline->GetHullModule(), vkGraphicsPipeline->GetDesc().hullEntryPoint));
    if (vkGraphicsPipeline->GetDomainModule()) stages.push_back(MakeStageInfo(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, vkGraphicsPipeline->GetDomainModule(), vkGraphicsPipeline->GetDesc().domainEntryPoint));
    if (vkGraphicsPipeline->GetGeometryModule()) stages.push_back(MakeStageInfo(VK_SHADER_STAGE_GEOMETRY_BIT, vkGraphicsPipeline->GetGeometryModule(), vkGraphicsPipeline->GetDesc().geometryEntryPoint));
    if (vkGraphicsPipeline->GetPixelModule()) stages.push_back(MakeStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, vkGraphicsPipeline->GetPixelModule(), vkGraphicsPipeline->GetDesc().pixelEntryPoint));

    // Vertex input: one binding per unique slot, AppendAligned offsets computed per slot.
    // If the caller didn't supply explicit elements, derive them from the vertex shader's
    // reflected input variables instead (mirrors D3D11's auto-derivation).
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    if (desc.numInputElements == 0)
    {
        ReflectVertexInputState(vkGraphicsPipeline->GetVertexSpirv(), bindings, attributes);
    }
    uint32_t runningOffsets[16] = {};
    bool slotSeen[16] = {};
    for (uint32_t i = 0; i < desc.numInputElements; ++i)
    {
        const InputElementDescription& element = desc.inputElements[i];
        uint32_t slot = element.slot < 16 ? element.slot : 0;

        uint32_t offset = element.alignedByteOffset == InputElementDescription::AppendAligned
            ? runningOffsets[slot]
            : element.alignedByteOffset;
        runningOffsets[slot] = offset + GetFormatByteSize(element.format);

        if (!slotSeen[slot])
        {
            slotSeen[slot] = true;
            VkVertexInputBindingDescription binding = {};
            binding.binding = slot;
            binding.stride = 0; // filled in below once all elements are processed
            binding.inputRate = element.classification == InputClassification::PerInstanceData ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
            bindings.push_back(binding);
        }

        VkVertexInputAttributeDescription attribute = {};
        attribute.location = i;
        attribute.binding = slot;
        attribute.format = ConvertFormat(element.format);
        attribute.offset = offset;
        attributes.push_back(attribute);
    }
    if (desc.numInputElements > 0)
    {
        for (auto& binding : bindings)
        {
            binding.stride = runningOffsets[binding.binding];
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    vertexInputState.pVertexBindingDescriptions = bindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInputState.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = ConvertTopology(desc.primitiveTopology);

    VkPipelineTessellationStateCreateInfo tessellationState = {};
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState.patchControlPoints = GetPatchControlPoints(desc.primitiveTopology);

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = desc.rasterizer.depthClipEnable ? VK_FALSE : VK_TRUE;
    rasterizationState.polygonMode = ConvertFillMode(desc.rasterizer.fillMode);
    rasterizationState.cullMode = ConvertCullMode(desc.rasterizer.cullMode);
    // SetViewport always uses a negative-height viewport (VK_KHR_maintenance1) to match
    // D3D's Y-down-in-screen-space convention; that already fully compensates for winding,
    // so frontFace maps straight across from the D3D-style desc with no extra inversion.
    rasterizationState.frontFace = desc.rasterizer.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthBiasEnable = desc.rasterizer.depthBias != 0 || desc.rasterizer.slopeScaledDepthBias != 0.0f ? VK_TRUE : VK_FALSE;
    rasterizationState.depthBiasConstantFactor = static_cast<float>(desc.rasterizer.depthBias);
    rasterizationState.depthBiasClamp = desc.rasterizer.depthBiasClamp;
    rasterizationState.depthBiasSlopeFactor = desc.rasterizer.slopeScaledDepthBias;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = ConvertSampleCount(desc.sampleCount);
    multisampleState.sampleShadingEnable = desc.rasterizer.multisampleEnable ? VK_TRUE : VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = desc.depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
    depthStencilState.depthWriteEnable = desc.depthStencil.depthWriteMask == DepthWriteMask::All ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp = ConvertCompareOp(desc.depthStencil.depthFunc);
    depthStencilState.stencilTestEnable = desc.depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
    depthStencilState.front = ConvertStencilOpState(desc.depthStencil.frontFace, desc.depthStencil.stencilReadMask, desc.depthStencil.stencilWriteMask, desc.stencilRef);
    depthStencilState.back = ConvertStencilOpState(desc.depthStencil.backFace, desc.depthStencil.stencilReadMask, desc.depthStencil.stencilWriteMask, desc.stencilRef);

    std::vector<VkPipelineColorBlendAttachmentState> attachments(desc.numRenderTargets);
    for (uint32_t i = 0; i < desc.numRenderTargets; ++i)
    {
        const RenderTargetBlendDescription& rt = desc.blend.renderTargets[i];
        VkPipelineColorBlendAttachmentState& att = attachments[i];
        att.blendEnable = rt.isBlendEnabled ? VK_TRUE : VK_FALSE;
        att.srcColorBlendFactor = ConvertBlend(rt.sourceBlend);
        att.dstColorBlendFactor = ConvertBlend(rt.destinationBlend);
        att.colorBlendOp = ConvertBlendOp(rt.blendOp);
        att.srcAlphaBlendFactor = ConvertBlend(rt.sourceBlendAlpha);
        att.dstAlphaBlendFactor = ConvertBlend(rt.destinationBlendAlpha);
        att.alphaBlendOp = ConvertBlendOp(rt.blendOpAlpha);
        att.colorWriteMask = ConvertColorWriteMask(rt.renderTargetWriteMask);
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = desc.numRenderTargets > 0 && desc.blend.renderTargets[0].isLogicOpEnabled ? VK_TRUE : VK_FALSE;
    colorBlendState.logicOp = desc.numRenderTargets > 0 ? ConvertLogicOp(desc.blend.renderTargets[0].logicOp) : VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = static_cast<uint32_t>(attachments.size());
    colorBlendState.pAttachments = attachments.data();
    colorBlendState.blendConstants[0] = desc.blendFactor.r;
    colorBlendState.blendConstants[1] = desc.blendFactor.g;
    colorBlendState.blendConstants[2] = desc.blendFactor.b;
    colorBlendState.blendConstants[3] = desc.blendFactor.a;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(std::size(dynamicStates));
    dynamicState.pDynamicStates = dynamicStates;

    std::vector<VkFormat> colorFormats(desc.numRenderTargets);
    for (uint32_t i = 0; i < desc.numRenderTargets; ++i)
    {
        colorFormats[i] = ConvertFormat(desc.renderTargetFormats[i]);
    }

    VkPipelineRenderingCreateInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
    renderingInfo.pColorAttachmentFormats = colorFormats.data();
    VkFormat depthFormat = ConvertFormat(desc.depthStencilFormat);
    if (IsDepthStencilFormat(depthFormat))
    {
        renderingInfo.depthAttachmentFormat = depthFormat;
        if (depthFormat == VK_FORMAT_D24_UNORM_S8_UINT || depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D16_UNORM_S8_UINT)
        {
            renderingInfo.stencilAttachmentFormat = depthFormat;
        }
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pTessellationState = tessellationState.patchControlPoints > 0 ? &tessellationState : nullptr;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout;

    valid = vkCreateGraphicsPipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, device->GetAllocationCallbacks(), &vkPipeline) == VK_SUCCESS;
}

VulkanComputePipelineState::VulkanComputePipelineState(VulkanDevice* device, const PrismObj<ComputePipeline>& pipeline, const ComputePipelineStateDesc& desc)
    : VulkanDeviceChild(device), ComputePipelineState(pipeline, desc)
{
    Create();
}

VulkanComputePipelineState::~VulkanComputePipelineState()
{
    VkDevice vkDevice = device->GetDevice();
    const VkAllocationCallbacks* alloc = device->GetAllocationCallbacks();
    if (vkPipeline) vkDestroyPipeline(vkDevice, vkPipeline, alloc);
    if (layout) vkDestroyPipelineLayout(vkDevice, layout, alloc);
}

ResourceBindingList& VulkanComputePipelineState::GetBindings() { return *bindingList; }

void VulkanComputePipelineState::Create()
{
    auto* vkComputePipeline = static_cast<VulkanComputePipeline*>(pipeline.Get());
    if (!vkComputePipeline || !vkComputePipeline->IsValid())
    {
        return;
    }

    std::vector<VulkanReflectedStage> reflectedStages;
    if (vkComputePipeline->GetComputeSpirv())
    {
        reflectedStages.push_back({ reinterpret_cast<const uint32_t*>(vkComputePipeline->GetComputeSpirv()->GetData()), vkComputePipeline->GetComputeSpirv()->GetLength(), VK_SHADER_STAGE_COMPUTE_BIT });
    }
    bindingList = std::make_unique<VulkanResourceBindingList>(device, pipeline.Get(), reflectedStages);

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast<uint32_t>(bindingList->GetSetLayouts().size());
    layoutInfo.pSetLayouts = bindingList->GetSetLayouts().data();
    if (vkCreatePipelineLayout(device->GetDevice(), &layoutInfo, device->GetAllocationCallbacks(), &layout) != VK_SUCCESS)
    {
        return;
    }

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = MakeStageInfo(VK_SHADER_STAGE_COMPUTE_BIT, vkComputePipeline->GetComputeModule(), vkComputePipeline->GetDesc().computeEntryPoint);
    pipelineInfo.layout = layout;

    valid = vkCreateComputePipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, device->GetAllocationCallbacks(), &vkPipeline) == VK_SUCCESS;
}

PrismObj<GraphicsPipeline> VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    auto pipeline = MakePrismObj<VulkanGraphicsPipeline>(this, desc);
    if (!pipeline->IsValid())
    {
        return {};
    }
    return pipeline;
}

PrismObj<GraphicsPipelineState> VulkanDevice::CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc)
{
    if (!pipeline)
    {
        return {};
    }
    auto state = MakePrismObj<VulkanGraphicsPipelineState>(this, PrismObj<GraphicsPipeline>(pipeline), desc);
    if (!state->IsValid())
    {
        return {};
    }
    return state;
}

PrismObj<ComputePipeline> VulkanDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
    auto pipeline = MakePrismObj<VulkanComputePipeline>(this, desc);
    if (!pipeline->IsValid())
    {
        return {};
    }
    return pipeline;
}

PrismObj<ComputePipelineState> VulkanDevice::CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc)
{
    if (!pipeline)
    {
        return {};
    }
    auto state = MakePrismObj<VulkanComputePipelineState>(this, PrismObj<ComputePipeline>(pipeline), desc);
    if (!state->IsValid())
    {
        return {};
    }
    return state;
}

PrismObj<SwapChain> VulkanDevice::CreateSwapChain(void* windowHandle, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc)
{
    SDL_Window* sdlWindow = static_cast<SDL_Window*>(windowHandle);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(sdlWindow, instance, GetAllocationCallbacks(), &surface))
    {
        std::cout << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << std::endl;
        return {};
    }

    auto swapChain = MakePrismObj<VulkanSwapChain>(this, windowHandle, surface, desc, fullscreenDesc);
    if (!swapChain->GetSwapchain())
    {
        return {};
    }

    return swapChain;
}

PrismObj<SwapChain> VulkanDevice::CreateSwapChain(void* windowHandle)
{
    SDL_Window* sdlWindow = static_cast<SDL_Window*>(windowHandle);
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(sdlWindow, &width, &height);

    SwapChainDesc desc = {};
    desc.width = static_cast<uint32_t>(width);
    desc.height = static_cast<uint32_t>(height);
    desc.format = Format::B8G8R8A8UNorm;
    desc.stereo = false;
    desc.sampleDesc = { 1, 0 };
    desc.bufferUsage = Usage::RenderTargetOutput;
    desc.bufferCount = 2;
    desc.scaling = Scaling::Stretch;
    desc.swapEffect = SwapEffect::FlipDiscard;
    desc.alphaMode = AlphaMode::Unspecified;
    desc.flags = SwapChainFlags::AllowModeSwitch;

    SwapChainFullscreenDesc fullscreenDesc = {};
    fullscreenDesc.windowed = true;
    fullscreenDesc.refreshRate = { 0, 1 };
    fullscreenDesc.scaling = Scaling::None;
    fullscreenDesc.scanlineOrdering = ScanlineOrder::Unspecified;

    return CreateSwapChain(windowHandle, desc, fullscreenDesc);
}

VulkanSwapChain::VulkanSwapChain(VulkanDevice* device, void* windowHandle, VkSurfaceKHR surface, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc)
    : SwapChain(desc, fullscreenDesc), device(device), windowHandle(windowHandle), surface(surface), swapchain(VK_NULL_HANDLE),
      vkFormat(VK_FORMAT_UNDEFINED), acquireFence(VK_NULL_HANDLE), currentImageIndex(0), imageAcquired(false)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(device->GetDevice(), &fenceInfo, device->GetAllocationCallbacks(), &acquireFence);

    CreateOrResizeSwapchain(desc.width, desc.height, desc.format, desc.bufferCount);
}

VulkanSwapChain::~VulkanSwapChain()
{
    DestroySwapchainResources();

    if (acquireFence)
    {
        vkDestroyFence(device->GetDevice(), acquireFence, device->GetAllocationCallbacks());
    }
    if (surface)
    {
        vkDestroySurfaceKHR(device->GetInstance(), surface, device->GetAllocationCallbacks());
    }
}

bool VulkanSwapChain::CreateOrResizeSwapchain(uint32_t width, uint32_t height, Format requestedFormat, uint32_t bufferCount)
{
    VkFormat format = ConvertFormat(requestedFormat);

    VkSurfaceCapabilitiesKHR caps;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice(), surface, &caps) != VK_SUCCESS)
    {
        std::cout << "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed" << std::endl;
        return false;
    }

    uint32_t imageCount = bufferCount;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    {
        imageCount = caps.maxImageCount;
    }
    if (imageCount < caps.minImageCount)
    {
        imageCount = caps.minImageCount;
    }

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == 0xFFFFFFFF)
    {
        extent.width = std::clamp(width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = format;
    swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = caps.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // always supported; simplest correct choice (vsync'd)
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = swapchain;

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(device->GetDevice(), &swapchainInfo, device->GetAllocationCallbacks(), &newSwapchain);

    DestroySwapchainResources(); // safe now: the new swapchain (if created) already references the old one via oldSwapchain

    if (result != VK_SUCCESS)
    {
        std::cout << "vkCreateSwapchainKHR failed: VkResult " << result << std::endl;
        return false;
    }

    swapchain = newSwapchain;
    vkFormat = format;

    uint32_t actualImageCount = 0;
    vkGetSwapchainImagesKHR(device->GetDevice(), swapchain, &actualImageCount, nullptr);
    std::vector<VkImage> images(actualImageCount);
    vkGetSwapchainImagesKHR(device->GetDevice(), swapchain, &actualImageCount, images.data());

    Texture2DDesc texDesc = {};
    texDesc.width = extent.width;
    texDesc.height = extent.height;
    texDesc.arraySize = 1;
    texDesc.mipLevels = 1;
    texDesc.sampleDesc = { 1, 0 };
    texDesc.gpuAccessFlags = GpuAccessFlags::Write;
    texDesc.format = requestedFormat;

    buffers.reserve(actualImageCount);
    for (VkImage image : images)
    {
        buffers.push_back(MakePrismObj<VulkanTexture2D>(device, texDesc, image, static_cast<VmaAllocation>(VK_NULL_HANDLE)));
    }

    desc.width = extent.width;
    desc.height = extent.height;
    desc.bufferCount = actualImageCount;
    desc.format = requestedFormat;

    imageAcquired = false;
    return true;
}

void VulkanSwapChain::DestroySwapchainResources()
{
    buffers.clear();
    if (swapchain)
    {
        vkDestroySwapchainKHR(device->GetDevice(), swapchain, device->GetAllocationCallbacks());
        swapchain = VK_NULL_HANDLE;
    }
}

void VulkanSwapChain::ResizeBuffers(uint32_t bufferCount, uint32_t width, uint32_t height, Format newFormat, SwapChainFlags swapChainFlags)
{
    vkDeviceWaitIdle(device->GetDevice());
    CreateOrResizeSwapchain(width, height, newFormat, bufferCount);
}

void VulkanSwapChain::EnsureImageAcquired()
{
    if (imageAcquired)
    {
        return;
    }

    VkResult result = vkAcquireNextImageKHR(device->GetDevice(), swapchain, UINT64_MAX, VK_NULL_HANDLE, acquireFence, &currentImageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        return;
    }

    vkWaitForFences(device->GetDevice(), 1, &acquireFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->GetDevice(), 1, &acquireFence);
    imageAcquired = true;
}

PrismObj<Texture2D> VulkanSwapChain::GetBuffer(size_t index)
{
    // Plain by-index accessor (no acquire side effect) so callers can fetch every
    // swapchain image once up front, e.g. to build one RTV per buffer, instead of
    // recreating views every frame. Use GetCurrentBackBufferIndex() to know which
    // one to actually render into this frame.
    return PrismObj<Texture2D>(buffers[index].Get());
}

uint32_t VulkanSwapChain::GetCurrentBackBufferIndex()
{
    EnsureImageAcquired();
    return currentImageIndex;
}

void VulkanSwapChain::Present(uint32_t interval, PresentFlags flags)
{
    if (!swapchain || !imageAcquired)
    {
        return;
    }

    const PrismObj<VulkanQueueStore>& graphicsQueue = device->GetGraphicsQueueStore();
    std::unique_lock queueLock(graphicsQueue->mutex);

    // v1: fully serialize so no explicit render-finished semaphore is needed yet.
    vkQueueWaitIdle(graphicsQueue->queue);

    VulkanTexture2D* currentBuffer = buffers[currentImageIndex].Get();
    if (currentBuffer->GetCurrentLayout() != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        VkCommandBufferAllocateInfo cmdAllocInfo = {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.commandPool = device->GetCommandPool();
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        if (vkAllocateCommandBuffers(device->GetDevice(), &cmdAllocInfo, &cmd) == VK_SUCCESS)
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &beginInfo);

            TransitionImage(cmd, currentBuffer->GetImage(), currentBuffer->GetCurrentLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);
            currentBuffer->SetCurrentLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmd;
            vkQueueSubmit(graphicsQueue->queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphicsQueue->queue);

            vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &cmd);
        }
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &currentImageIndex;

    vkQueuePresentKHR(graphicsQueue->queue, &presentInfo);
    queueLock.unlock();

    imageAcquired = false;
}

PrismObj<Query> VulkanDevice::CreateQuery(const QueryDesc& desc)
{
    return {};
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer(device->GetAllocator(), buffer, allocation);
}

VulkanTexture1D::~VulkanTexture1D()
{
    if (allocation) // null for swapchain-owned images, which the swapchain destroys itself
    {
        vmaDestroyImage(device->GetAllocator(), image, allocation);
    }
}

VulkanTexture2D::~VulkanTexture2D()
{
    if (allocation) // null for swapchain-owned images, which the swapchain destroys itself
    {
        vmaDestroyImage(device->GetAllocator(), image, allocation);
    }
}

VulkanTexture3D::~VulkanTexture3D()
{
    if (allocation) // null for swapchain-owned images, which the swapchain destroys itself
    {
        vmaDestroyImage(device->GetAllocator(), image, allocation);
    }
}

VulkanFence::~VulkanFence()
{
    vkDestroySemaphore(device->GetDevice(), semaphore, device->GetAllocationCallbacks());
}

void VulkanFence::Signal(uint64_t value)
{
    VkSemaphoreSignalInfo signalInfo = {};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    signalInfo.semaphore = semaphore;
    signalInfo.value = value;
    vkSignalSemaphore(device->GetDevice(), &signalInfo);
}

uint64_t VulkanFence::GetCompletedValue() const
{
    uint64_t value = 0;
    vkGetSemaphoreCounterValue(device->GetDevice(), semaphore, &value);
    return value;
}

bool VulkanFence::Wait(uint64_t value, uint64_t timeoutNs)
{
    VkSemaphoreWaitInfo waitInfo = {};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &semaphore;
    waitInfo.pValues = &value;
    return vkWaitSemaphores(device->GetDevice(), &waitInfo, timeoutNs) == VK_SUCCESS;
}

void VulkanCommandQueue::Submit(CommandList** lists, uint32_t count, Fence* fence, uint64_t signalValue)
{
    VkCommandBufferSubmitInfo* cmdInfos = reinterpret_cast<VkCommandBufferSubmitInfo*>(alloca(sizeof(VkCommandBufferSubmitInfo) * count));
    for (uint32_t i = 0; i < count; ++i)
    {
        cmdInfos[i] = {};
        cmdInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfos[i].commandBuffer = static_cast<VulkanCommandList*>(lists[i])->GetCommandBuffer();
    }

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.commandBufferInfoCount = count;
    submitInfo.pCommandBufferInfos = cmdInfos;

    VkSemaphoreSubmitInfo signalInfo = {};
    if (fence)
    {
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfo.semaphore = static_cast<VulkanFence*>(fence)->GetSemaphore();
        signalInfo.value = signalValue;
        signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &signalInfo;
    }

    std::lock_guard lock(queue->mutex);
    vkQueueSubmit2(queue->queue, 1, &submitInfo, VK_NULL_HANDLE);
}

void VulkanCommandQueue::WaitIdle()
{
    std::lock_guard lock(queue->mutex);
    vkQueueWaitIdle(queue->queue);
}

VulkanCommandAllocator::~VulkanCommandAllocator()
{
    vkDestroyCommandPool(device->GetDevice(), commandPool, device->GetAllocationCallbacks());
}

bool VulkanCommandAllocator::Reset()
{
    VK_CHK(vkResetCommandPool(device->GetDevice(), commandPool, 0));
    return true;
}

void VulkanCommandList::EnsureDrawBegin()
{
    if (drawing) return;

    VkRenderingInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    VkRenderingAttachmentInfo colorAtt[VkMaxSimultaneousRenderTargets]{};
    VulkanRenderTargetView* firstRtv = nullptr;

    VkRect2D rect = {};
    rect.offset.x = rect.offset.y = std::numeric_limits<int32_t>::max();
    for (auto i = 0; i < VkMaxSimultaneousRenderTargets; ++i)
    {
        auto* rtv = state.rtvs[i];
        auto& viewport = state.viewports[i];

        if (rtv)
        {
            TransitionResourceTo(commandBuffer, rtv->GetResource(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

            auto& att = colorAtt[info.colorAttachmentCount];
            if (!firstRtv)
            {
                firstRtv = rtv;
            }
            att.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            att.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            att.imageView = rtv->GetImageView();
            att.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            rect.offset.x = std::min(rect.offset.x, static_cast<int32_t>(viewport.x));
            rect.offset.y = std::min(rect.offset.y, static_cast<int32_t>(viewport.y));
            rect.extent.width = std::max(rect.extent.width, static_cast<uint32_t>(viewport.x + viewport.width));
            rect.extent.height = std::max(rect.extent.height, static_cast<uint32_t>(viewport.y + viewport.height));
            ++info.colorAttachmentCount;
        }
    }

    if (!firstRtv)
    {
        return;
    }

    drawing = true;

    info.renderArea.offset = rect.offset;
    info.renderArea.extent = { (uint32_t)(rect.extent.width - rect.offset.x), (uint32_t)(rect.extent.height - rect.offset.y) };

    VkRenderingAttachmentInfo depthAtt{};
    if (state.dsv)
    {
        VkFormat dsvFormat = ConvertFormat(state.dsv->GetDesc().format);
        TransitionResourceTo(commandBuffer, state.dsv->GetResource(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, ConvertAspectFlags(dsvFormat));

        depthAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAtt.imageView = state.dsv->GetImageView();
        depthAtt.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        info.pDepthAttachment = &depthAtt;
    }

    auto& desc = firstRtv->GetDesc();
    switch (desc.dimension)
    {
    case RenderTargetViewDimension::Texture1DArray:
        info.layerCount = desc.texture1DArray.arraySize;
        break;
    case RenderTargetViewDimension::Texture2DArray:
        info.layerCount = desc.texture2DArray.arraySize;
        break;
    case RenderTargetViewDimension::Texture2DMSArray:
        info.layerCount = desc.texture2DMSArray.arraySize;
        break;
    case RenderTargetViewDimension::Texture3D:
    case RenderTargetViewDimension::Texture1D:
    case RenderTargetViewDimension::Texture2D:
    case RenderTargetViewDimension::Texture2DMS:
    case RenderTargetViewDimension::Buffer:
    default:
        info.layerCount = 1;
        break;
    }

    info.pColorAttachments = colorAtt;
    vkCmdBeginRendering(commandBuffer, &info);

    vkCmdSetPrimitiveTopology(commandBuffer, ConvertTopology(state.primitiveTopology));

    if (state.state)
    {
        if (auto* gfx = dynamic_cast<VulkanGraphicsPipelineState*>(state.state))
        {
            if (gfx->IsValid())
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx->GetVkPipeline());
                BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx->GetLayout(), gfx->GetBindings());
            }
        }
    }

    for (uint32_t i = 0; i < state.vertexBufferCount; ++i)
    {
        if (state.vertexBuffers[i])
        {
            VkBuffer vkBuffer = static_cast<VulkanBuffer*>(state.vertexBuffers[i])->GetBuffer();
            VkDeviceSize offset = state.vertexOffsets[i];
            vkCmdBindVertexBuffers(commandBuffer, i, 1, &vkBuffer, &offset);
        }
    }

    if (state.indexBuffer)
    {
        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(state.indexBuffer)->GetBuffer();
        VkIndexType indexType = state.indexFormat == Format::R16UInt ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(commandBuffer, vkBuffer, state.indexOffset, indexType);
    }
}

void VulkanCommandList::EnsureDrawEnd()
{
    if (!drawing) return;
    vkCmdEndRendering(commandBuffer);
    drawing = false;
}

PrismDevice* VulkanCommandList::GetDevice() const noexcept { return device; }

void VulkanCommandList::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void VulkanCommandList::End()
{
    EnsureDrawEnd();
    vkEndCommandBuffer(commandBuffer);
}

void VulkanCommandList::SetGraphicsPipelineState(GraphicsPipelineState* pipelineState)
{
    if (state.state != pipelineState)
    {
        EnsureDrawEnd();
    }
    state.state = pipelineState;
}

void VulkanCommandList::SetComputePipelineState(ComputePipelineState* pipelineState)
{
    auto* vkPso = static_cast<VulkanComputePipelineState*>(pipelineState);
    if (vkPso && vkPso->IsValid())
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPso->GetVkPipeline());
        BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPso->GetLayout(), vkPso->GetBindings());
    }
}

void VulkanCommandList::SetVertexBuffer(uint32_t slot, Buffer* buffer, uint32_t stride, uint32_t offset)
{
    if (slot >= 16) return;
    state.vertexBuffers[slot] = buffer;
    state.vertexStrides[slot] = stride;
    state.vertexOffsets[slot] = offset;
    if (slot >= state.vertexBufferCount)
    {
        state.vertexBufferCount = slot + 1;
    }

    if (drawing && buffer)
    {
        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetBuffer();
        VkDeviceSize vkOffset = offset;
        vkCmdBindVertexBuffers(commandBuffer, slot, 1, &vkBuffer, &vkOffset);
    }
}

void VulkanCommandList::SetIndexBuffer(Buffer* buffer, Format format, uint32_t offset)
{
    state.indexBuffer = buffer;
    state.indexOffset = offset;
    state.indexFormat = format;

    if (drawing && buffer)
    {
        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetBuffer();
        VkIndexType indexType = format == Format::R16UInt ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(commandBuffer, vkBuffer, offset, indexType);
    }
}

void VulkanCommandList::SetRenderTarget(RenderTargetView* rtv, DepthStencilView* dsv)
{
    if (state.rtvs[0] != static_cast<VulkanRenderTargetView*>(rtv) || state.dsv != static_cast<VulkanDepthStencilView*>(dsv))
    {
        EnsureDrawEnd();
    }
    for (uint32_t i = 1; i < VkMaxSimultaneousRenderTargets; ++i)
    {
        state.rtvs[i] = nullptr;
    }
    state.rtvs[0] = static_cast<VulkanRenderTargetView*>(rtv);
    state.dsv = static_cast<VulkanDepthStencilView*>(dsv);
}

void VulkanCommandList::SetRenderTargetsAndUnorderedAccessViews(uint32_t count, RenderTargetView** views, DepthStencilView* depthStencilView, uint32_t uavSlot, uint32_t uavCount, UnorderedAccessView** uavs, uint32_t* pUavInitialCount)
{
    EnsureDrawEnd();
    for (uint32_t i = 0; i < VkMaxSimultaneousRenderTargets; ++i)
    {
        state.rtvs[i] = i < count ? static_cast<VulkanRenderTargetView*>(views[i]) : nullptr;
    }
    state.dsv = static_cast<VulkanDepthStencilView*>(depthStencilView);

    if (uavCount == 0)
    {
        return;
    }

    auto* gfx = dynamic_cast<VulkanGraphicsPipelineState*>(state.state);
    if (!gfx || !gfx->IsValid())
    {
        return;
    }

    auto& bindings = static_cast<VulkanResourceBindingList&>(gfx->GetBindings());
    for (uint32_t i = 0; i < uavCount; ++i)
    {
        auto* uav = static_cast<VulkanUnorderedAccessView*>(uavs[i]);
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView = uav ? uav->GetImageView() : VK_NULL_HANDLE;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        if (uav)
        {
            TransitionResourceTo(commandBuffer, uav->GetResource(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        bindings.TrySetPixelUAVBySlot(device, uavSlot + i, uav, &imageInfo);
    }
}

void VulkanCommandList::SetViewport(const Viewport& viewport)
{
    state.viewports[0] = viewport;

    VkViewport vp = {};
    vp.x = viewport.x;
    vp.y = viewport.y + viewport.height; // flip Y: match D3D's top-left, Y-down NDC convention
    vp.width = viewport.width;
    vp.height = -viewport.height;
    vp.minDepth = viewport.minDepth;
    vp.maxDepth = viewport.maxDepth;
    vkCmdSetViewport(commandBuffer, 0, 1, &vp);

    if (state.scissorCount == 0)
    {
        VkRect2D scissor = {};
        scissor.offset = { static_cast<int32_t>(viewport.x), static_cast<int32_t>(viewport.y) };
        scissor.extent = { static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height) };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
}

void VulkanCommandList::SetViewports(uint32_t viewportCount, const Viewport* viewports)
{
    if (viewportCount == 0 || !viewports) return;

    std::vector<VkViewport> vkViewports(viewportCount);
    for (uint32_t i = 0; i < viewportCount; ++i)
    {
        if (i < VkMaxSimultaneousRenderTargets)
        {
            state.viewports[i] = viewports[i];
        }
        vkViewports[i].x = viewports[i].x;
        vkViewports[i].y = viewports[i].y + viewports[i].height;
        vkViewports[i].width = viewports[i].width;
        vkViewports[i].height = -viewports[i].height;
        vkViewports[i].minDepth = viewports[i].minDepth;
        vkViewports[i].maxDepth = viewports[i].maxDepth;
    }
    vkCmdSetViewport(commandBuffer, 0, viewportCount, vkViewports.data());
}

void VulkanCommandList::SetPrimitiveTopology(PrimitiveTopology topology)
{
    state.primitiveTopology = topology;
    if (drawing)
    {
        vkCmdSetPrimitiveTopology(commandBuffer, ConvertTopology(topology));
    }
}

void VulkanCommandList::SetScissorRects(const Rect* rects, uint32_t rectCount)
{
    if (rectCount == 0 || !rects) return;

    std::vector<VkRect2D> scissors(rectCount);
    for (uint32_t i = 0; i < rectCount; ++i)
    {
        scissors[i].offset = { rects[i].left, rects[i].top };
        scissors[i].extent = { static_cast<uint32_t>(rects[i].right - rects[i].left), static_cast<uint32_t>(rects[i].bottom - rects[i].top) };
        if (i < VkMaxSimultaneousRenderTargets)
        {
            state.scissors[i] = rects[i];
        }
    }
    state.scissorCount = rectCount;
    vkCmdSetScissor(commandBuffer, 0, rectCount, scissors.data());
}

void VulkanCommandList::DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset)
{
    EnsureDrawBegin();
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
}

void VulkanCommandList::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, int32_t vertexOffset, uint32_t instanceOffset)
{
    EnsureDrawBegin();
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}

void VulkanCommandList::DrawIndexedInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs)
{
    EnsureDrawBegin();
    vkCmdDrawIndexedIndirect(commandBuffer, static_cast<VulkanBuffer*>(bufferForArgs)->GetBuffer(), alignedByteOffsetForArgs, 1, 0);
}

void VulkanCommandList::DrawInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs)
{
    EnsureDrawBegin();
    vkCmdDrawIndirect(commandBuffer, static_cast<VulkanBuffer*>(bufferForArgs)->GetBuffer(), alignedByteOffsetForArgs, 1, 0);
}

void VulkanCommandList::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    EnsureDrawEnd();
    vkCmdDispatch(commandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void VulkanCommandList::DispatchIndirect(Buffer* dispatchArgs, uint32_t offset)
{
    EnsureDrawEnd();
    vkCmdDispatchIndirect(commandBuffer, static_cast<VulkanBuffer*>(dispatchArgs)->GetBuffer(), offset);
}

void VulkanCommandList::ExecuteCommandList(CommandList* commandList)
{
    // Bundles (secondary command buffers) aren't modeled yet; nothing to execute.
}

void VulkanCommandList::ClearRenderTargetView(RenderTargetView* rtv, const Color& color)
{
    if (!rtv) return;
    auto* vkRtv = static_cast<VulkanRenderTargetView*>(rtv);

    bool wasDrawing = drawing;
    EnsureDrawEnd();

    TransitionResourceTo(commandBuffer, vkRtv->GetResource(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    VkClearColorValue clearColor = {};
    clearColor.float32[0] = color.r;
    clearColor.float32[1] = color.g;
    clearColor.float32[2] = color.b;
    clearColor.float32[3] = color.a;

    VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
    vkCmdClearColorImage(commandBuffer, GetVkImage(vkRtv->GetResource()), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

    if (wasDrawing)
    {
        EnsureDrawBegin();
    }
}

void VulkanCommandList::ClearDepthStencilView(DepthStencilView* dsv, DepthStencilViewClearFlags flags, float depth, char stencil)
{
    if (!dsv) return;
    auto* vkDsv = static_cast<VulkanDepthStencilView*>(dsv);

    bool wasDrawing = drawing;
    EnsureDrawEnd();

    VkFormat format = ConvertFormat(vkDsv->GetDesc().format);
    VkImageAspectFlags aspect = ConvertAspectFlags(format);
    TransitionResourceTo(commandBuffer, vkDsv->GetResource(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aspect);

    VkClearDepthStencilValue clearValue = {};
    clearValue.depth = depth;
    clearValue.stencil = static_cast<uint32_t>(stencil);

    VkImageSubresourceRange range = { aspect, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
    vkCmdClearDepthStencilImage(commandBuffer, GetVkImage(vkDsv->GetResource()), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &range);

    if (wasDrawing)
    {
        EnsureDrawBegin();
    }
}

void VulkanCommandList::ClearUnorderedAccessViewUint(UnorderedAccessView* uav, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    if (!uav) return;
    auto* vkUav = static_cast<VulkanUnorderedAccessView*>(uav);

    bool wasDrawing = drawing;
    EnsureDrawEnd();

    TransitionResourceTo(commandBuffer, vkUav->GetResource(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

    VkClearColorValue clearColor = {};
    clearColor.uint32[0] = r;
    clearColor.uint32[1] = g;
    clearColor.uint32[2] = b;
    clearColor.uint32[3] = a;

    VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS };
    vkCmdClearColorImage(commandBuffer, GetVkImage(vkUav->GetResource()), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);

    if (wasDrawing)
    {
        EnsureDrawBegin();
    }
}

void VulkanCommandList::ClearView(ResourceView* view, const Color& color, const Rect& rect)
{
    VkRect2D vkRect = {};
    vkRect.offset.x = rect.left;
    vkRect.offset.y = rect.top;
    vkRect.extent.width = static_cast<uint32_t>(rect.right - rect.left);
    vkRect.extent.height = static_cast<uint32_t>(rect.bottom - rect.top);

    VkClearRect clearRect = {};
    clearRect.rect = vkRect;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    if (auto* rtv = dynamic_cast<VulkanRenderTargetView*>(view))
    {
        bool wasDrawing = drawing;
        EnsureDrawEnd();

        TransitionResourceTo(commandBuffer, rtv->GetResource(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

        VkRenderingAttachmentInfo colorAtt = {};
        colorAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAtt.imageView = rtv->GetImageView();
        colorAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.renderArea = vkRect;
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAtt;

        vkCmdBeginRendering(commandBuffer, &renderInfo);

        VkClearAttachment clearAtt = {};
        clearAtt.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAtt.colorAttachment = 0;
        clearAtt.clearValue.color = { color.r, color.g, color.b, color.a };

        vkCmdClearAttachments(commandBuffer, 1, &clearAtt, 1, &clearRect);

        vkCmdEndRendering(commandBuffer);

        if (wasDrawing)
        {
            EnsureDrawBegin();
        }
        return;
    }

    if (auto* dsv = dynamic_cast<VulkanDepthStencilView*>(view))
    {
        bool wasDrawing = drawing;
        EnsureDrawEnd();

        VkFormat dsvFormat = ConvertFormat(dsv->GetDesc().format);
        VkImageAspectFlags aspect = ConvertAspectFlags(dsvFormat);
        TransitionResourceTo(commandBuffer, dsv->GetResource(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspect);

        VkRenderingAttachmentInfo depthAtt = {};
        depthAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAtt.imageView = dsv->GetImageView();
        depthAtt.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderInfo = {};
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.renderArea = vkRect;
        renderInfo.layerCount = 1;
        if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            renderInfo.pDepthAttachment = &depthAtt;
        }
        if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            renderInfo.pStencilAttachment = &depthAtt;
        }

        vkCmdBeginRendering(commandBuffer, &renderInfo);

        VkClearAttachment clearAtt = {};
        clearAtt.aspectMask = aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
        clearAtt.clearValue.depthStencil = { color.r, 0 };

        vkCmdClearAttachments(commandBuffer, 1, &clearAtt, 1, &clearRect);

        vkCmdEndRendering(commandBuffer);

        if (wasDrawing)
        {
            EnsureDrawBegin();
        }
        return;
    }

    if (auto* uav = dynamic_cast<VulkanUnorderedAccessView*>(view))
    {
        bool wasDrawing = drawing;
        EnsureDrawEnd();

        TransitionResourceTo(commandBuffer, uav->GetResource(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

        VkDescriptorSet descriptorSet = device->GetNextClearUAVDescriptorSet();

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView = uav->GetImageView();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet;
        write.dstBinding = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);

        struct
        {
            int32_t offsetX, offsetY;
            uint32_t extentX, extentY;
            float color[4];
        } pushConstants;
        pushConstants.offsetX = rect.left;
        pushConstants.offsetY = rect.top;
        pushConstants.extentX = static_cast<uint32_t>(rect.right - rect.left);
        pushConstants.extentY = static_cast<uint32_t>(rect.bottom - rect.top);
        pushConstants.color[0] = color.r;
        pushConstants.color[1] = color.g;
        pushConstants.color[2] = color.b;
        pushConstants.color[3] = color.a;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, device->GetClearUAVPipeline());
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, device->GetClearUAVPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
        vkCmdPushConstants(commandBuffer, device->GetClearUAVPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants), &pushConstants);
        vkCmdDispatch(commandBuffer, (pushConstants.extentX + 7) / 8, (pushConstants.extentY + 7) / 8, 1);

        if (wasDrawing)
        {
            EnsureDrawBegin();
        }
    }
}

void VulkanCommandList::CopyResource(Resource* dstResource, Resource* srcResource)
{
    if (!dstResource || !srcResource) return;

    auto* srcBuffer = dynamic_cast<VulkanBuffer*>(srcResource);
    auto* dstBuffer = dynamic_cast<VulkanBuffer*>(dstResource);
    if (srcBuffer && dstBuffer)
    {
        EnsureDrawEnd();
        VkBufferCopy region = {};
        region.size = std::min(srcBuffer->GetDesc().widthInBytes, dstBuffer->GetDesc().widthInBytes);
        vkCmdCopyBuffer(commandBuffer, srcBuffer->GetBuffer(), dstBuffer->GetBuffer(), 1, &region);
        return;
    }

    VkImage srcImage = GetVkImage(srcResource);
    VkImage dstImage = GetVkImage(dstResource);
    if (srcImage != VK_NULL_HANDLE && dstImage != VK_NULL_HANDLE)
    {
        EnsureDrawEnd();
        TransitionResourceTo(commandBuffer, srcResource, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
        TransitionResourceTo(commandBuffer, dstResource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

        auto* dstTex = dynamic_cast<VulkanTexture2D*>(dstResource);
        if (dstTex)
        {
            VkImageCopy region = {};
            region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            region.extent = { dstTex->GetDesc().width, dstTex->GetDesc().height, 1 };
            vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }
    }
}

void VulkanCommandList::GenerateMips(ShaderResourceView* srv)
{
    // Requires walking mip chains with per-level blits; not implemented yet.
}

void VulkanCommandList::ClearState()
{
    EnsureDrawEnd();
    state = CommandListState{};
}

void VulkanCommandList::Flush()
{
    EnsureDrawEnd();
}

MappedSubresource VulkanCommandList::Map(Resource* resource, uint32_t subresource, MapType mapType, MapFlags mapFlags)
{
    MappedSubresource result = {};
    auto* buffer = dynamic_cast<VulkanBuffer*>(resource);
    if (!buffer)
    {
        return result;
    }

    void* data = nullptr;
    if (vmaMapMemory(device->GetAllocator(), buffer->GetAllocation(), &data) == VK_SUCCESS)
    {
        result.data = data;
        result.rowPitch = buffer->GetDesc().widthInBytes;
        result.depthPitch = buffer->GetDesc().widthInBytes;
    }
    return result;
}

void VulkanCommandList::Unmap(Resource* resource, uint32_t subresource)
{
    auto* buffer = dynamic_cast<VulkanBuffer*>(resource);
    if (!buffer)
    {
        return;
    }
    vmaUnmapMemory(device->GetAllocator(), buffer->GetAllocation());
}

void VulkanCommandList::BeginQuery(Query* query)
{
    // Query pool management isn't implemented yet.
}

void VulkanCommandList::EndQuery(Query* query)
{
}

bool VulkanCommandList::QueryGetData(Query* query, void* data, uint32_t size, QueryGetDataFlags flags)
{
    return false;
}

void VulkanCommandList::BeginEvent(const char* name)
{
}

void VulkanCommandList::EndEvent()
{
}


PrismDevice* VulkanDeviceChild::GetDevice() { return device; }

PrismDevice* VulkanBuffer::GetDevice() const noexcept { return device; }
PrismDevice* VulkanTexture1D::GetDevice() const noexcept { return device; }
PrismDevice* VulkanTexture2D::GetDevice() const noexcept { return device; }
PrismDevice* VulkanTexture3D::GetDevice() const noexcept { return device; }
PrismDevice* VulkanFence::GetDevice() const noexcept { return device; }
PrismDevice* VulkanCommandQueue::GetDevice() const noexcept { return device; }
PrismDevice* VulkanCommandAllocator::GetDevice() const noexcept { return device; }
PrismDevice* VulkanRenderTargetView::GetDevice() const noexcept { return device; }
PrismDevice* VulkanDepthStencilView::GetDevice() const noexcept { return device; }
PrismDevice* VulkanShaderResourceView::GetDevice() const noexcept { return device; }
PrismDevice* VulkanUnorderedAccessView::GetDevice() const noexcept { return device; }
PrismDevice* VulkanSamplerState::GetDevice() const noexcept { return device; }

VulkanRenderTargetView::~VulkanRenderTargetView()
{
    vkDestroyImageView(device->GetDevice(), imageView, device->GetAllocationCallbacks());
}

VulkanDepthStencilView::~VulkanDepthStencilView()
{
    vkDestroyImageView(device->GetDevice(), imageView, device->GetAllocationCallbacks());
}

VulkanShaderResourceView::~VulkanShaderResourceView()
{
    vkDestroyImageView(device->GetDevice(), imageView, device->GetAllocationCallbacks());
}

VulkanUnorderedAccessView::~VulkanUnorderedAccessView()
{
    vkDestroyImageView(device->GetDevice(), imageView, device->GetAllocationCallbacks());
}

VulkanSamplerState::~VulkanSamplerState()
{
    vkDestroySampler(device->GetDevice(), sampler, device->GetAllocationCallbacks());
}


HEXA_PRISM_NAMESPACE_END



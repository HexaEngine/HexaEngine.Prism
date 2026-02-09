#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

HEXA_PRISM_NAMESPACE_BEGIN

#define VK_CHK(result) \
    if (result != VK_SUCCESS) { return false; }

bool VulkanGraphicsDevice::FindQueueFamily(QueueFamilyIndices& indices) const
{
    static constexpr auto InvalidIndex = QueueFamilyIndices::InvalidIndex;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, families.data());

    for (uint32_t i = 0; i < count; i++)
    {
        auto flags = families[i].queueFlags;
        if ((flags & VK_QUEUE_GRAPHICS_BIT) && indices.graphics == InvalidIndex)
        {
            indices.graphics = i;
        }

        if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && indices.compute == InvalidIndex)
        {
            indices.compute = i;
        }

        if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT) && indices.transfer == InvalidIndex)
        {
            indices.transfer = i;
        }
    }

    if (indices.compute == InvalidIndex)
    {
        indices.compute = indices.graphics;
    }
    if (indices.transfer == InvalidIndex)
    {
        indices.transfer = indices.graphics;
    }

    return indices.graphics != InvalidIndex;
}

bool VulkanGraphicsDevice::CreateLogicalDevice()
{
    if (!FindQueueFamily(queueIndicies))
        return false;

    std::unordered_set<uint32_t> uniqueFamilies = { queueIndicies.graphics };
    if (queueIndicies.compute != QueueFamilyIndices::InvalidIndex)
        uniqueFamilies.insert(queueIndicies.compute);
    if (queueIndicies.transfer != QueueFamilyIndices::InvalidIndex)
        uniqueFamilies.insert(queueIndicies.transfer);

    float priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (uint32_t family : uniqueFamilies)
    {
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
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

    vkGetDeviceQueue(device, queueIndicies.graphics, 0, &graphicsQueue);
    if (queueIndicies.compute != QueueFamilyIndices::InvalidIndex)
    {
        vkGetDeviceQueue(device, queueIndicies.compute, 0, &computeQueue);
    }
    if (queueIndicies.transfer != QueueFamilyIndices::InvalidIndex)
    {
        vkGetDeviceQueue(device, queueIndicies.transfer, 0, &transferQueue);
    }

    return true;
}

bool VulkanGraphicsDevice::Initialize(GraphicsDeviceFlags flags)
{
    std::vector<const char*> layers;
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

    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if HEXA_PRISM_WINDOWS
    if ((flags & GraphicsDeviceFlags::Win32) != 0)
    {
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#endif

#if HEXA_PRISM_LINUX
    if ((flags & GraphicsDeviceFlags::X11) != 0)
    {
        extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    }
    if ((flags & GraphicsDeviceFlags::Wayland) != 0)
    {
        extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
    }
#endif

    VkApplicationInfo appInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "HexaEngine Prism",
        .apiVersion = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VkResult result;

    VK_CHK(vkCreateInstance(&createInfo, NULL, &instance));

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

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueIndicies.graphics;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHK(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1; 

    VkCommandBuffer commandBuffer;
    VK_CHK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    immediateCommandList = MakePrismObj<VulkanCommandList>(this, commandBuffer, CommandListType::Direct);

    return true;
}

CommandList* VulkanGraphicsDevice::GetImmediateCommandList()
{
    return immediateCommandList.Get();
}

PrismObj<Buffer> VulkanGraphicsDevice::CreateBuffer(const BufferDesc& desc, const SubresourceData* initialData)
{
    return {};
}

PrismObj<Texture1D> VulkanGraphicsDevice::CreateTexture1D(const Texture1DDesc& desc)
{
    return {};
}

PrismObj<Texture2D> VulkanGraphicsDevice::CreateTexture2D(const Texture2DDesc& desc)
{
    return {};
}

PrismObj<Texture3D> VulkanGraphicsDevice::CreateTexture3D(const Texture3DDesc& desc)
{
    return {};
}

PrismObj<RenderTargetView> VulkanGraphicsDevice::CreateRenderTargetView(Resource* resource, const RenderTargetViewDesc& desc)
{
    return {};
}

PrismObj<ShaderResourceView> VulkanGraphicsDevice::CreateShaderResourceView(Resource* resource, const ShaderResourceViewDesc& desc)
{
    return {};
}

PrismObj<DepthStencilView> VulkanGraphicsDevice::CreateDepthStencilView(Resource* resource, const DepthStencilViewDesc& desc)
{
    return {};
}

PrismObj<UnorderedAccessView> VulkanGraphicsDevice::CreateUnorderedAccessView(Resource* resource, const UnorderedAccessViewDesc& desc)
{
    return {};
}

PrismObj<SamplerState> VulkanGraphicsDevice::CreateSamplerState(const SamplerDesc& desc)
{
    return {};
}

PrismObj<CommandList> VulkanGraphicsDevice::CreateCommandList()
{
    return {};
}

PrismObj<GraphicsPipeline> VulkanGraphicsDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    return {};
}

PrismObj<GraphicsPipelineState> VulkanGraphicsDevice::CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc)
{
    return {};
}

PrismObj<ComputePipeline> VulkanGraphicsDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
    return {};
}

PrismObj<ComputePipelineState> VulkanGraphicsDevice::CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc)
{
    return {};
}

PrismObj<SwapChain> VulkanGraphicsDevice::CreateSwapChain(void* windowHandle, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc)
{
    return {};
}

PrismObj<SwapChain> VulkanGraphicsDevice::CreateSwapChain(void* windowHandle)
{
    return {};
}

PrismObj<Query> VulkanGraphicsDevice::CreateQuery(const QueryDesc& desc)
{
    return {};
}

void VulkanCommandList::EnsureDrawBegin()
{
    if (drawing) return;
    drawing = true;
 
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

    info.renderArea.offset = rect.offset;
    info.renderArea.extent = { (uint32_t)(rect.extent.width - rect.offset.x), (uint32_t)(rect.extent.height - rect.offset.y) };

    VkRenderingAttachmentInfo depthAtt{};
    if (state.dsv)
    {
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
}

void VulkanCommandList::EnsureDrawEnd()
{
    if (!drawing) return;
    vkCmdEndRendering(commandBuffer);
    drawing = false;
}

void VulkanCommandList::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void VulkanCommandList::End()
{
    vkEndCommandBuffer(commandBuffer);
}


GraphicsDevice* VulkanDeviceChild::GetDevice() { return device; }


HEXA_PRISM_NAMESPACE_END

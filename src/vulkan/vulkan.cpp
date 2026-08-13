#define VMA_IMPLEMENTATION
#include "vulkan/vulkan.hpp"
#include "vulkan/resource_binding_list.hpp"
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <SDL3/SDL_vulkan.h>

HEXA_PRISM_NAMESPACE_BEGIN

#define VK_CHK(result) \
    if (result != VK_SUCCESS) { return false; }

#define VK_CHKRETDEF(result) \
    if (result != VK_SUCCESS) { return {}; }

namespace
{
    VkFormat ConvertFormat(const Format format)
    {
        switch (format)
        {
        case Format::Unknown: return VK_FORMAT_UNDEFINED;
        case Format::R32G32B32A32Typeless: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::R32G32B32A32Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::R32G32B32A32UInt: return VK_FORMAT_R32G32B32A32_UINT;
        case Format::R32G32B32A32SInt: return VK_FORMAT_R32G32B32A32_SINT;
        case Format::R32G32B32Typeless: return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::R32G32B32Float: return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::R32G32B32UInt: return VK_FORMAT_R32G32B32_UINT;
        case Format::R32G32B32SInt: return VK_FORMAT_R32G32B32_SINT;
        case Format::R16G16B16A16Typeless: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::R16G16B16A16Float: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::R16G16B16A16UNorm: return VK_FORMAT_R16G16B16A16_UNORM;
        case Format::R16G16B16A16UInt: return VK_FORMAT_R16G16B16A16_UINT;
        case Format::R16G16B16A16SNorm: return VK_FORMAT_R16G16B16A16_SNORM;
        case Format::R16G16B16A16Sint: return VK_FORMAT_R16G16B16A16_SINT;
        case Format::R32G32Typeless: return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32Float: return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32UInt: return VK_FORMAT_R32G32_UINT;
        case Format::R32G32SInt: return VK_FORMAT_R32G32_SINT;
        case Format::R32G8X24Typeless: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::D32FloatS8X24UInt: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::R32FloatX8X24Typeless: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::X32TypelessG8X24UInt: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case Format::R10G10B10A2Typeless: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case Format::R10G10B10A2UNorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case Format::R10G10B10A2UInt: return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case Format::R11G11B10Float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case Format::R8G8B8A8Typeless: return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::R8G8B8A8UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::R8G8B8A8UNormSRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::R8G8B8A8UInt: return VK_FORMAT_R8G8B8A8_UINT;
        case Format::R8G8B8A8SNorm: return VK_FORMAT_R8G8B8A8_SNORM;
        case Format::R8G8B8A8SInt: return VK_FORMAT_R8G8B8A8_SINT;
        case Format::R16G16Typeless: return VK_FORMAT_R16G16_SFLOAT;
        case Format::R16G16Float: return VK_FORMAT_R16G16_SFLOAT;
        case Format::R16G16UNorm: return VK_FORMAT_R16G16_UNORM;
        case Format::R16G16UInt: return VK_FORMAT_R16G16_UINT;
        case Format::R16G16SNorm: return VK_FORMAT_R16G16_SNORM;
        case Format::R16G16Sint: return VK_FORMAT_R16G16_SINT;
        case Format::R32Typeless: return VK_FORMAT_R32_SFLOAT;
        case Format::D32Float: return VK_FORMAT_D32_SFLOAT;
        case Format::R32Float: return VK_FORMAT_R32_SFLOAT;
        case Format::R32UInt: return VK_FORMAT_R32_UINT;
        case Format::R32SInt: return VK_FORMAT_R32_SINT;
        case Format::R24G8Typeless: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::D24UNormS8UInt: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::R24UNormX8Typeless: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::X24TypelessG8UInt: return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::R8G8Typeless: return VK_FORMAT_R8G8_UNORM;
        case Format::R8G8UNorm: return VK_FORMAT_R8G8_UNORM;
        case Format::R8G8UInt: return VK_FORMAT_R8G8_UINT;
        case Format::R8G8SNorm: return VK_FORMAT_R8G8_SNORM;
        case Format::R8G8Sint: return VK_FORMAT_R8G8_SINT;
        case Format::R16Typeless: return VK_FORMAT_R16_SFLOAT;
        case Format::D16UNorm: return VK_FORMAT_D16_UNORM;
        case Format::R16UNorm: return VK_FORMAT_R16_UNORM;
        case Format::R16UInt: return VK_FORMAT_R16_UINT;
        case Format::R16SNorm: return VK_FORMAT_R16_SNORM;
        case Format::R16Sint: return VK_FORMAT_R16_SINT;
        case Format::R8Typeless: return VK_FORMAT_R8_UNORM;
        case Format::R8UNorm: return VK_FORMAT_R8_UNORM;
        case Format::R8UInt: return VK_FORMAT_R8_UINT;
        case Format::R8SNorm: return VK_FORMAT_R8_SNORM;
        case Format::R8SInt: return VK_FORMAT_R8_SINT;
        case Format::A8UNorm: return VK_FORMAT_R8_UNORM;
        case Format::R9G9B9E5SharedExp: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case Format::BC1Typeless: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case Format::BC1UNorm: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case Format::BC1UNormSRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case Format::BC2Typeless: return VK_FORMAT_BC2_UNORM_BLOCK;
        case Format::BC2UNorm: return VK_FORMAT_BC2_UNORM_BLOCK;
        case Format::BC2UNormSRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
        case Format::BC3Typeless: return VK_FORMAT_BC3_UNORM_BLOCK;
        case Format::BC3UNorm: return VK_FORMAT_BC3_UNORM_BLOCK;
        case Format::BC3UNormSRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
        case Format::BC4Typeless: return VK_FORMAT_BC4_UNORM_BLOCK;
        case Format::BC4UNorm: return VK_FORMAT_BC4_UNORM_BLOCK;
        case Format::BC4SNorm: return VK_FORMAT_BC4_SNORM_BLOCK;
        case Format::BC5Typeless: return VK_FORMAT_BC5_UNORM_BLOCK;
        case Format::BC5UNorm: return VK_FORMAT_BC5_UNORM_BLOCK;
        case Format::BC5SNorm: return VK_FORMAT_BC5_SNORM_BLOCK;
        case Format::B5G6R5UNorm: return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case Format::B5G5R5A1UNorm: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case Format::B8G8R8A8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::B8G8R8X8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::R10G10B10XRBiasA2UNorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case Format::B8G8R8A8Typeless: return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::B8G8R8A8UNormSRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::B8G8R8X8Typeless: return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::B8G8R8X8UNormSRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::BC6HTypeless: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case Format::BC6HUF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case Format::BC6HSF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case Format::BC7Typeless: return VK_FORMAT_BC7_UNORM_BLOCK;
        case Format::BC7UNorm: return VK_FORMAT_BC7_UNORM_BLOCK;
        case Format::BC7UNormSRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
        case Format::B4G4R4A4UNorm: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        default: return VK_FORMAT_UNDEFINED; // video/legacy formats with no Vulkan core equivalent
        }
    }

    bool IsDepthStencilFormat(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return true;
        default:
            return false;
        }
    }

    VkImageAspectFlags ConvertAspectFlags(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    VkImageUsageFlags ConvertImageUsageFlags(GpuAccessFlags flags, VkFormat format)
    {
        VkImageUsageFlags result = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if ((flags & GpuAccessFlags::Read) != GpuAccessFlags::None)
        {
            result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if ((flags & GpuAccessFlags::UA) != GpuAccessFlags::None)
        {
            result |= VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if ((flags & GpuAccessFlags::DepthStencil) != GpuAccessFlags::None || IsDepthStencilFormat(format))
        {
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        else if ((flags & GpuAccessFlags::Write) != GpuAccessFlags::None)
        {
            result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        return result;
    }

    VkBufferUsageFlags ConvertBufferUsageFlags(const BufferDesc& desc)
    {
        VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        switch (desc.type)
        {
        case BufferType::ConstantBuffer:
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BufferType::VertexBuffer:
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferType::IndexBuffer:
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        default:
            break;
        }

        if ((desc.gpuAccessFlags & (GpuAccessFlags::Read | GpuAccessFlags::UA)) != GpuAccessFlags::None)
        {
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        return result;
    }

    VmaAllocationCreateInfo ConvertAllocationInfo(CpuAccessFlags cpuAccess, GpuAccessFlags)
    {
        VmaAllocationCreateInfo info = {};
        info.usage = VMA_MEMORY_USAGE_AUTO;

        const bool cpuRead = (cpuAccess & CpuAccessFlags::Read) != CpuAccessFlags::None;
        const bool cpuWrite = (cpuAccess & CpuAccessFlags::Write) != CpuAccessFlags::None;

        if (cpuRead)
        {
            info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        else if (cpuWrite)
        {
            info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        return info;
    }

    VkSampleCountFlagBits ConvertSampleCount(uint32_t count)
    {
        switch (count)
        {
        case 1: return VK_SAMPLE_COUNT_1_BIT;
        case 2: return VK_SAMPLE_COUNT_2_BIT;
        case 4: return VK_SAMPLE_COUNT_4_BIT;
        case 8: return VK_SAMPLE_COUNT_8_BIT;
        case 16: return VK_SAMPLE_COUNT_16_BIT;
        case 32: return VK_SAMPLE_COUNT_32_BIT;
        case 64: return VK_SAMPLE_COUNT_64_BIT;
        default: return VK_SAMPLE_COUNT_1_BIT;
        }
    }

    VkImage GetVkImage(Resource* resource)
    {
        if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) return tex->GetImage();
        if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) return tex->GetImage();
        if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) return tex->GetImage();
        return VK_NULL_HANDLE;
    }

    // D3D convention: Format::Unknown in a view desc means "inherit the resource's own format".
    Format GetResourceFormat(Resource* resource)
    {
        if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) return tex->GetDesc().format;
        if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) return tex->GetDesc().format;
        if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) return tex->GetDesc().format;
        return Format::Unknown;
    }

    Format ResolveViewFormat(Resource* resource, Format requested)
    {
        return requested == Format::Unknown ? GetResourceFormat(resource) : requested;
    }

    VkImageLayout GetTrackedLayout(Resource* resource)
    {
        if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) return tex->GetCurrentLayout();
        if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) return tex->GetCurrentLayout();
        if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) return tex->GetCurrentLayout();
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    void SetTrackedLayout(Resource* resource, VkImageLayout layout)
    {
        if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) { tex->SetCurrentLayout(layout); return; }
        if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) { tex->SetCurrentLayout(layout); return; }
        if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) { tex->SetCurrentLayout(layout); return; }
    }

    // Coarse (ALL_COMMANDS) barrier: correct, not finely scheduled. Matches the rest of this
    // backend's "synchronous and correct first, optimize later" approach.
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

    VkFilter ConvertMinMagFilter(Filter filter)
    {
        switch (filter)
        {
        case Filter::MinMagMipPoint:
        case Filter::MinMagPointMipLinear:
        case Filter::ComparisonMinMagMipPoint:
        case Filter::ComparisonMinMagPointMipLinear:
        case Filter::MinimumMinMagMipPoint:
        case Filter::MinimumMinMagPointMipLinear:
        case Filter::MaximumMinMagMipPoint:
        case Filter::MaximumMinMagPointMipLinear:
            return VK_FILTER_NEAREST;
        default:
            return VK_FILTER_LINEAR;
        }
    }

    VkSamplerMipmapMode ConvertMipmapMode(Filter filter)
    {
        switch (filter)
        {
        case Filter::MinMagMipPoint:
        case Filter::MinPointMagLinearMipPoint:
        case Filter::MinLinearMagMipPoint:
        case Filter::MinMagLinearMipPoint:
        case Filter::ComparisonMinMagMipPoint:
        case Filter::ComparisonMinPointMagLinearMipPoint:
        case Filter::ComparisonMinLinearMagMipPoint:
        case Filter::ComparisonMinMagLinearMipPoint:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        default:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    bool IsAnisotropicFilter(Filter filter)
    {
        switch (filter)
        {
        case Filter::Anisotropic:
        case Filter::ComparisonAnisotropic:
        case Filter::MinimumAnisotropic:
        case Filter::MaximumAnisotropic:
            return true;
        default:
            return false;
        }
    }

    bool IsComparisonFilter(Filter filter)
    {
        return static_cast<uint32_t>(filter) >= static_cast<uint32_t>(Filter::ComparisonMinMagMipPoint)
            && static_cast<uint32_t>(filter) <= static_cast<uint32_t>(Filter::ComparisonAnisotropic);
    }

    VkSamplerAddressMode ConvertAddressMode(TextureAddressMode mode)
    {
        switch (mode)
        {
        case TextureAddressMode::Wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TextureAddressMode::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TextureAddressMode::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TextureAddressMode::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case TextureAddressMode::MirrorOnce: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkCompareOp ConvertCompareOp(ComparisonFunc func)
    {
        switch (func)
        {
        case ComparisonFunc::Never: return VK_COMPARE_OP_NEVER;
        case ComparisonFunc::Less: return VK_COMPARE_OP_LESS;
        case ComparisonFunc::Equal: return VK_COMPARE_OP_EQUAL;
        case ComparisonFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case ComparisonFunc::Greater: return VK_COMPARE_OP_GREATER;
        case ComparisonFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case ComparisonFunc::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case ComparisonFunc::Always: return VK_COMPARE_OP_ALWAYS;
        default: return VK_COMPARE_OP_ALWAYS;
        }
    }

    bool UploadBufferData(VulkanDevice* device, VkBuffer dstBuffer, const void* data, size_t size)
    {
        VkBufferCreateInfo stagingInfo = {};
        stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingInfo.size = size;
        stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo = {};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;
        VmaAllocationInfo stagingAllocInfoOut;
        if (vmaCreateBuffer(device->GetAllocator(), &stagingInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocInfoOut) != VK_SUCCESS)
        {
            return false;
        }

        PrismMemoryCopy(stagingAllocInfoOut.pMappedData, data, size);

        VkCommandBufferAllocateInfo cmdAllocInfo = {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.commandPool = device->GetCommandPool();
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        if (vkAllocateCommandBuffers(device->GetDevice(), &cmdAllocInfo, &cmd) != VK_SUCCESS)
        {
            vmaDestroyBuffer(device->GetAllocator(), stagingBuffer, stagingAllocation);
            return false;
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(cmd, stagingBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        VkQueue queue = device->GetGraphicsQueue();
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &cmd);
        vmaDestroyBuffer(device->GetAllocator(), stagingBuffer, stagingAllocation);
        return true;
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

bool VulkanDevice::CreateLogicalDevice()
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

bool VulkanDevice::Initialize(const DeviceDesc& desc)
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
    poolInfo.queueFamilyIndex = queueIndicies.graphics;
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

    return true;
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
    poolInfo.queueFamilyIndex = queueIndicies.graphics;
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

PrismObj<Texture1D> VulkanDevice::CreateTexture1D(const Texture1DDesc& desc)
{
    VkFormat format = ConvertFormat(desc.format);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_1D;
    imageInfo.format = format;
    imageInfo.extent = { desc.width, 1, 1 };
    imageInfo.mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;
    imageInfo.arrayLayers = desc.arraySize == 0 ? 1 : desc.arraySize;
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

    return MakePrismObj<VulkanTexture1D>(this, desc, image, imageAllocation);
}

PrismObj<Texture2D> VulkanDevice::CreateTexture2D(const Texture2DDesc& desc)
{
    VkFormat format = ConvertFormat(desc.format);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { desc.width, desc.height, 1 };
    imageInfo.mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;
    imageInfo.arrayLayers = desc.arraySize == 0 ? 1 : desc.arraySize;
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

    return MakePrismObj<VulkanTexture2D>(this, desc, image, imageAllocation);
}

PrismObj<Texture3D> VulkanDevice::CreateTexture3D(const Texture3DDesc& desc)
{
    VkFormat format = ConvertFormat(desc.format);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.format = format;
    imageInfo.extent = { desc.width, desc.height, desc.depth };
    imageInfo.mipLevels = desc.mipLevels == 0 ? 1 : desc.mipLevels;
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

    return MakePrismObj<VulkanTexture3D>(this, desc, image, imageAllocation);
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
    viewInfo.format = ConvertFormat(desc.format);
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

    VkFormat format = ConvertFormat(desc.format);

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

    VkFormat format = ConvertFormat(desc.format);

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
    viewInfo.format = ConvertFormat(desc.format);
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
    VkPrimitiveTopology ConvertTopology(PrimitiveTopology topology)
    {
        switch (topology)
        {
        case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::LineListAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::LineStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleListAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        default:
            if (static_cast<uint32_t>(topology) >= static_cast<uint32_t>(PrimitiveTopology::PatchListWith1ControlPoints))
            {
                return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            }
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    uint32_t GetPatchControlPoints(PrimitiveTopology topology)
    {
        uint32_t value = static_cast<uint32_t>(topology);
        uint32_t base = static_cast<uint32_t>(PrimitiveTopology::PatchListWith1ControlPoints);
        return value >= base ? value - base + 1 : 0;
    }

    VkPolygonMode ConvertFillMode(FillMode mode)
    {
        return mode == FillMode::Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    }

    VkCullModeFlags ConvertCullMode(CullMode mode)
    {
        switch (mode)
        {
        case CullMode::None: return VK_CULL_MODE_NONE;
        case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
        default: return VK_CULL_MODE_NONE;
        }
    }

    VkBlendFactor ConvertBlend(Blend blend)
    {
        switch (blend)
        {
        case Blend::Zero: return VK_BLEND_FACTOR_ZERO;
        case Blend::One: return VK_BLEND_FACTOR_ONE;
        case Blend::SourceColor: return VK_BLEND_FACTOR_SRC_COLOR;
        case Blend::InverseSourceColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case Blend::SourceAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
        case Blend::InverseSourceAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case Blend::DestinationAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
        case Blend::InverseDestinationAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case Blend::DestinationColor: return VK_BLEND_FACTOR_DST_COLOR;
        case Blend::InverseDestinationColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case Blend::SourceAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case Blend::BlendFactor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case Blend::InverseBlendFactor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case Blend::Source1Color: return VK_BLEND_FACTOR_SRC1_COLOR;
        case Blend::InverseSource1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case Blend::Source1Alpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
        case Blend::InverseSource1Alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default: return VK_BLEND_FACTOR_ONE;
        }
    }

    VkBlendOp ConvertBlendOp(BlendOperation op)
    {
        switch (op)
        {
        case BlendOperation::Add: return VK_BLEND_OP_ADD;
        case BlendOperation::Subtract: return VK_BLEND_OP_SUBTRACT;
        case BlendOperation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOperation::Min: return VK_BLEND_OP_MIN;
        case BlendOperation::Max: return VK_BLEND_OP_MAX;
        default: return VK_BLEND_OP_ADD;
        }
    }

    VkLogicOp ConvertLogicOp(LogicOperation op)
    {
        switch (op)
        {
        case LogicOperation::Clear: return VK_LOGIC_OP_CLEAR;
        case LogicOperation::Set: return VK_LOGIC_OP_SET;
        case LogicOperation::Copy: return VK_LOGIC_OP_COPY;
        case LogicOperation::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOperation::Noop: return VK_LOGIC_OP_NO_OP;
        case LogicOperation::Invert: return VK_LOGIC_OP_INVERT;
        case LogicOperation::And: return VK_LOGIC_OP_AND;
        case LogicOperation::Nand: return VK_LOGIC_OP_NAND;
        case LogicOperation::Or: return VK_LOGIC_OP_OR;
        case LogicOperation::Nor: return VK_LOGIC_OP_NOR;
        case LogicOperation::Xor: return VK_LOGIC_OP_XOR;
        case LogicOperation::Equiv: return VK_LOGIC_OP_EQUIVALENT;
        case LogicOperation::AndReverse: return VK_LOGIC_OP_AND_REVERSE;
        case LogicOperation::AndInverted: return VK_LOGIC_OP_AND_INVERTED;
        case LogicOperation::OrReverse: return VK_LOGIC_OP_OR_REVERSE;
        case LogicOperation::OrInverted: return VK_LOGIC_OP_OR_INVERTED;
        default: return VK_LOGIC_OP_COPY;
        }
    }

    VkStencilOp ConvertStencilOp(StencilOperation op)
    {
        switch (op)
        {
        case StencilOperation::Keep: return VK_STENCIL_OP_KEEP;
        case StencilOperation::Zero: return VK_STENCIL_OP_ZERO;
        case StencilOperation::Replace: return VK_STENCIL_OP_REPLACE;
        case StencilOperation::IncrementSaturate: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOperation::DecrementSaturate: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOperation::Invert: return VK_STENCIL_OP_INVERT;
        case StencilOperation::Increment: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOperation::Decrement: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default: return VK_STENCIL_OP_KEEP;
        }
    }

    VkStencilOpState ConvertStencilOpState(const DepthStencilOperationDescription& stencilDesc, uint8_t readMask, uint8_t writeMask, uint32_t reference)
    {
        VkStencilOpState state = {};
        state.failOp = ConvertStencilOp(stencilDesc.stencilFailOp);
        state.passOp = ConvertStencilOp(stencilDesc.stencilPassOp);
        state.depthFailOp = ConvertStencilOp(stencilDesc.stencilDepthFailOp);
        state.compareOp = ConvertCompareOp(stencilDesc.stencilFunc);
        state.compareMask = readMask;
        state.writeMask = writeMask;
        state.reference = reference;
        return state;
    }

    VkColorComponentFlags ConvertColorWriteMask(ColorWriteEnable mask)
    {
        VkColorComponentFlags result = 0;
        uint8_t m = static_cast<uint8_t>(mask);
        if (m & static_cast<uint8_t>(ColorWriteEnable::Red)) result |= VK_COLOR_COMPONENT_R_BIT;
        if (m & static_cast<uint8_t>(ColorWriteEnable::Green)) result |= VK_COLOR_COMPONENT_G_BIT;
        if (m & static_cast<uint8_t>(ColorWriteEnable::Blue)) result |= VK_COLOR_COMPONENT_B_BIT;
        if (m & static_cast<uint8_t>(ColorWriteEnable::Alpha)) result |= VK_COLOR_COMPONENT_A_BIT;
        return result;
    }

    uint32_t GetFormatByteSize(Format format)
    {
        switch (format)
        {
        case Format::R32G32B32A32Float:
        case Format::R32G32B32A32UInt:
        case Format::R32G32B32A32SInt:
            return 16;
        case Format::R32G32B32Float:
        case Format::R32G32B32UInt:
        case Format::R32G32B32SInt:
            return 12;
        case Format::R16G16B16A16Float:
        case Format::R16G16B16A16UNorm:
        case Format::R16G16B16A16UInt:
        case Format::R16G16B16A16SNorm:
        case Format::R16G16B16A16Sint:
        case Format::R32G32Float:
        case Format::R32G32UInt:
        case Format::R32G32SInt:
            return 8;
        case Format::R10G10B10A2UNorm:
        case Format::R10G10B10A2UInt:
        case Format::R11G11B10Float:
        case Format::R8G8B8A8UNorm:
        case Format::R8G8B8A8UNormSRGB:
        case Format::R8G8B8A8UInt:
        case Format::R8G8B8A8SNorm:
        case Format::R8G8B8A8SInt:
        case Format::R16G16Float:
        case Format::R16G16UNorm:
        case Format::R16G16UInt:
        case Format::R16G16SNorm:
        case Format::R16G16Sint:
        case Format::R32Float:
        case Format::R32UInt:
        case Format::R32SInt:
        case Format::B8G8R8A8UNorm:
        case Format::B8G8R8X8UNorm:
            return 4;
        case Format::R8G8UNorm:
        case Format::R8G8UInt:
        case Format::R8G8SNorm:
        case Format::R8G8Sint:
        case Format::R16UNorm:
        case Format::R16UInt:
        case Format::R16SNorm:
        case Format::R16Sint:
            return 2;
        case Format::R8UNorm:
        case Format::R8UInt:
        case Format::R8SNorm:
        case Format::R8SInt:
            return 1;
        default:
            return 4;
        }
    }

    bool CompileAndCreateModule(VulkanDevice* device, const PrismObj<ShaderSource>& source, const char* entryPoint, ShaderStage stage, VkShaderModule& moduleOut, PrismObj<Blob>& spirvOut)
    {
        if (!source)
        {
            return true; // optional stage
        }

        if (!VulkanShaderCompiler::Compile(source.Get(), entryPoint, stage, spirvOut))
        {
            return false;
        }

        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleInfo.codeSize = spirvOut->GetLength();
        moduleInfo.pCode = reinterpret_cast<const uint32_t*>(spirvOut->GetData());

        return vkCreateShaderModule(device->GetDevice(), &moduleInfo, device->GetAllocationCallbacks(), &moduleOut) == VK_SUCCESS;
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
    success &= CompileAndCreateModule(device, desc.vertexShader, desc.vertexEntryPoint, ShaderStage::Vertex, vertexModule, vertexSpirv);
    success &= CompileAndCreateModule(device, desc.hullShader, desc.hullEntryPoint, ShaderStage::Hull, hullModule, hullSpirv);
    success &= CompileAndCreateModule(device, desc.domainShader, desc.domainEntryPoint, ShaderStage::Domain, domainModule, domainSpirv);
    success &= CompileAndCreateModule(device, desc.geometryShader, desc.geometryEntryPoint, ShaderStage::Geometry, geometryModule, geometrySpirv);
    success &= CompileAndCreateModule(device, desc.pixelShader, desc.pixelEntryPoint, ShaderStage::Pixel, pixelModule, pixelSpirv);
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
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
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
    for (auto& binding : bindings)
    {
        binding.stride = runningOffsets[binding.binding];
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

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
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

    CreateOrResizeSwapchain(desc.width, desc.height, ConvertFormat(desc.format), desc.bufferCount);
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

bool VulkanSwapChain::CreateOrResizeSwapchain(uint32_t width, uint32_t height, VkFormat format, uint32_t bufferCount)
{
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

    buffers.reserve(actualImageCount);
    for (VkImage image : images)
    {
        buffers.push_back(MakePrismObj<VulkanTexture2D>(device, texDesc, image, static_cast<VmaAllocation>(VK_NULL_HANDLE)));
    }

    desc.width = extent.width;
    desc.height = extent.height;
    desc.bufferCount = actualImageCount;

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
    CreateOrResizeSwapchain(width, height, ConvertFormat(newFormat), bufferCount);
}

PrismObj<Texture2D> VulkanSwapChain::GetBuffer(size_t index)
{
    if (!swapchain)
    {
        return {};
    }

    if (!imageAcquired)
    {
        VkResult result = vkAcquireNextImageKHR(device->GetDevice(), swapchain, UINT64_MAX, VK_NULL_HANDLE, acquireFence, &currentImageIndex);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            return {};
        }

        vkWaitForFences(device->GetDevice(), 1, &acquireFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device->GetDevice(), 1, &acquireFence);
        imageAcquired = true;
    }

    if (currentImageIndex >= buffers.size())
    {
        return {};
    }

    return PrismObj<Texture2D>(buffers[currentImageIndex].Get());
}

void VulkanSwapChain::Present(uint32_t interval, PresentFlags flags)
{
    if (!swapchain || !imageAcquired)
    {
        return;
    }

    // v1: fully serialize so no explicit render-finished semaphore is needed yet.
    vkQueueWaitIdle(device->GetGraphicsQueue());

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
            vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(device->GetGraphicsQueue());

            vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &cmd);
        }
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &currentImageIndex;

    vkQueuePresentKHR(device->GetGraphicsQueue(), &presentInfo);

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

    vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE);
}

void VulkanCommandQueue::WaitIdle()
{
    vkQueueWaitIdle(queue);
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
    // UAVs bound alongside render targets are resolved through the resource binding list, not
    // tracked here; nothing further to do until that path is exercised.
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
    // Baked into the PSO for this backend (VkPipelineInputAssemblyStateCreateInfo); nothing to
    // set dynamically here since VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY isn't opted into.
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
    // Partial-area clears aren't modeled yet (Vulkan needs a scissor'd draw or a render-area
    // limited clear, not a plain vkCmdClear*Image); fall back to a full clear of the same view.
    if (auto* rtv = dynamic_cast<RenderTargetView*>(view))
    {
        ClearRenderTargetView(rtv, color);
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



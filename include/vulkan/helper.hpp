#pragma once

#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

inline VkFormat ConvertFormat(const Format format)
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

inline bool IsDepthStencilFormat(VkFormat format)
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

inline VkImageAspectFlags ConvertAspectFlags(VkFormat format)
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

inline VkImageUsageFlags ConvertImageUsageFlags(GpuAccessFlags flags, VkFormat format)
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

inline VkBufferUsageFlags ConvertBufferUsageFlags(const BufferDesc& desc)
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

inline VmaAllocationCreateInfo ConvertAllocationInfo(CpuAccessFlags cpuAccess, GpuAccessFlags)
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

inline VkSampleCountFlagBits ConvertSampleCount(uint32_t count)
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

inline VkImage GetVkImage(Resource* resource)
{
    if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) return tex->GetImage();
    if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) return tex->GetImage();
    if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) return tex->GetImage();
    return VK_NULL_HANDLE;
}

// D3D convention: Format::Unknown in a view desc means "inherit the resource's own format".
inline Format GetResourceFormat(Resource* resource)
{
    if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) return tex->GetDesc().format;
    if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) return tex->GetDesc().format;
    if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) return tex->GetDesc().format;
    return Format::Unknown;
}

inline Format ResolveViewFormat(Resource* resource, Format requested)
{
    return requested == Format::Unknown ? GetResourceFormat(resource) : requested;
}

inline VkImageLayout GetTrackedLayout(Resource* resource)
{
    if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) return tex->GetCurrentLayout();
    if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) return tex->GetCurrentLayout();
    if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) return tex->GetCurrentLayout();
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

inline void SetTrackedLayout(Resource* resource, VkImageLayout layout)
{
    if (auto* tex = dynamic_cast<VulkanTexture2D*>(resource)) { tex->SetCurrentLayout(layout); return; }
    if (auto* tex = dynamic_cast<VulkanTexture1D*>(resource)) { tex->SetCurrentLayout(layout); return; }
    if (auto* tex = dynamic_cast<VulkanTexture3D*>(resource)) { tex->SetCurrentLayout(layout); return; }
}



inline VkFilter ConvertMinMagFilter(Filter filter)
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

inline VkSamplerMipmapMode ConvertMipmapMode(Filter filter)
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

inline bool IsAnisotropicFilter(Filter filter)
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

inline bool IsComparisonFilter(Filter filter)
{
    return static_cast<uint32_t>(filter) >= static_cast<uint32_t>(Filter::ComparisonMinMagMipPoint)
        && static_cast<uint32_t>(filter) <= static_cast<uint32_t>(Filter::ComparisonAnisotropic);
}

inline VkSamplerAddressMode ConvertAddressMode(TextureAddressMode mode)
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

inline VkCompareOp ConvertCompareOp(ComparisonFunc func)
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

inline VkShaderStageFlagBits ConvertStage(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderStage::Hull: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderStage::Domain: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderStage::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderStage::Pixel: return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
    default: return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

inline ShaderParameterType ConvertParameterType(VkDescriptorType type)
{
    switch (type)
    {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return ShaderParameterType::CBV;
    case VK_DESCRIPTOR_TYPE_SAMPLER: return ShaderParameterType::Sampler;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return ShaderParameterType::UAV;
    default: return ShaderParameterType::SRV;
    }
}

inline VkPrimitiveTopology ConvertTopology(PrimitiveTopology topology)
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

inline uint32_t GetPatchControlPoints(PrimitiveTopology topology)
{
    uint32_t value = static_cast<uint32_t>(topology);
    uint32_t base = static_cast<uint32_t>(PrimitiveTopology::PatchListWith1ControlPoints);
    return value >= base ? value - base + 1 : 0;
}

inline VkPolygonMode ConvertFillMode(FillMode mode)
{
    return mode == FillMode::Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
}

inline VkCullModeFlags ConvertCullMode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None: return VK_CULL_MODE_NONE;
    case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
    case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
    default: return VK_CULL_MODE_NONE;
    }
}

inline VkBlendFactor ConvertBlend(Blend blend)
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

inline VkBlendOp ConvertBlendOp(BlendOperation op)
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

inline VkLogicOp ConvertLogicOp(LogicOperation op)
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

inline VkStencilOp ConvertStencilOp(StencilOperation op)
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

inline VkStencilOpState ConvertStencilOpState(const DepthStencilOperationDescription& stencilDesc, uint8_t readMask, uint8_t writeMask, uint32_t reference)
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

inline VkColorComponentFlags ConvertColorWriteMask(ColorWriteEnable mask)
{
    VkColorComponentFlags result = 0;
    uint8_t m = static_cast<uint8_t>(mask);
    if (m & static_cast<uint8_t>(ColorWriteEnable::Red)) result |= VK_COLOR_COMPONENT_R_BIT;
    if (m & static_cast<uint8_t>(ColorWriteEnable::Green)) result |= VK_COLOR_COMPONENT_G_BIT;
    if (m & static_cast<uint8_t>(ColorWriteEnable::Blue)) result |= VK_COLOR_COMPONENT_B_BIT;
    if (m & static_cast<uint8_t>(ColorWriteEnable::Alpha)) result |= VK_COLOR_COMPONENT_A_BIT;
    return result;
}

inline uint32_t GetFormatByteSize(Format format)
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

HEXA_PRISM_NAMESPACE_END
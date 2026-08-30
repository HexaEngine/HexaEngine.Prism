#include "vulkan/resource_binding_list.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/helper.hpp"
#include <spirv_reflect.h>

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
    size_t StageToIndex(VkShaderStageFlagBits stage)
    {
        switch (stage)
        {
        case VK_SHADER_STAGE_VERTEX_BIT: return 0;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return 1;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return 2;
        case VK_SHADER_STAGE_GEOMETRY_BIT: return 3;
        case VK_SHADER_STAGE_FRAGMENT_BIT: return 4;
        case VK_SHADER_STAGE_COMPUTE_BIT: return 0;
        default: return 0;
        }
    }

    ShaderStage IndexToShaderStage(size_t index, bool isCompute)
    {
        if (isCompute)
        {
            return ShaderStage::Compute;
        }
        switch (index)
        {
        case 0: return ShaderStage::Vertex;
        case 1: return ShaderStage::Hull;
        case 2: return ShaderStage::Domain;
        case 3: return ShaderStage::Geometry;
        case 4: return ShaderStage::Pixel;
        default: return ShaderStage::Vertex;
        }
    }

}

VulkanResourceBindingList::VulkanResourceBindingList(VulkanDevice* device, Pipeline* pipeline, const std::vector<VulkanReflectedStage>& stages)
    : device(device), pipeline(pipeline)
{
    std::array<std::vector<VulkanShaderParameter>, StageCount> parametersByStage;
    size_t maxIndex = 0;
    bool isCompute = !stages.empty() && stages.front().stage == VK_SHADER_STAGE_COMPUTE_BIT;

    for (const auto& stageInfo : stages)
    {
        SpvReflectShaderModule module = {};
        if (spvReflectCreateShaderModule(stageInfo.codeSize, stageInfo.code, &module) != SPV_REFLECT_RESULT_SUCCESS)
        {
            continue;
        }

        uint32_t count = 0;
        spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
        std::vector<SpvReflectDescriptorBinding*> spvBindings(count);
        spvReflectEnumerateDescriptorBindings(&module, &count, spvBindings.data());

        size_t index = StageToIndex(stageInfo.stage);
        maxIndex = std::max(maxIndex, index);

        for (auto* binding : spvBindings)
        {
            const char* name = binding->name ? binding->name : "";
            VkDescriptorType descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type);

            VulkanShaderParameter parameter;
            parameter.name = name;
            parameter.hash = VulkanDescriptorRange::HashString(name);
            parameter.index = binding->binding;
            parameter.descriptorType = descriptorType;
            parameter.stage = IndexToShaderStage(index, isCompute);
            parameter.type = ConvertParameterType(descriptorType);
            parametersByStage[index].push_back(std::move(parameter));
        }

        spvReflectDestroyShaderModule(&module);
    }

    for (size_t index = 0; index < StageCount; ++index)
    {
        std::vector<VulkanShaderParameter>& parameters = parametersByStage[index];
        ranges[index] = VulkanDescriptorRange(IndexToShaderStage(index, isCompute), parameters.data(), static_cast<int>(parameters.size()));
    }

    VkDevice vkDevice = device->GetDevice();
    for (size_t index = 0; index <= maxIndex; ++index)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings = ranges[index].BuildLayoutBindings();

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, device->GetAllocationCallbacks(), &layout);
        setLayouts.push_back(layout);
    }

    if (!setLayouts.empty())
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = device->GetDescriptorPool();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(setLayouts.size());
        allocInfo.pSetLayouts = setLayouts.data();

        descriptorSets.resize(setLayouts.size());
        vkAllocateDescriptorSets(vkDevice, &allocInfo, descriptorSets.data());

        for (size_t index = 0; index < descriptorSets.size(); ++index)
        {
            ranges[index].descriptorSet = descriptorSets[index];
        }
    }
}

VulkanResourceBindingList::~VulkanResourceBindingList()
{
    VkDevice vkDevice = device->GetDevice();
    if (!descriptorSets.empty())
    {
        vkFreeDescriptorSets(vkDevice, device->GetDescriptorPool(), static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
    }
    for (VkDescriptorSetLayout layout : setLayouts)
    {
        vkDestroyDescriptorSetLayout(vkDevice, layout, device->GetAllocationCallbacks());
    }
}

void VulkanResourceBindingList::SetCBV(const char* name, Buffer* buffer)
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer ? static_cast<VulkanBuffer*>(buffer)->GetBuffer() : VK_NULL_HANDLE;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    for (auto& range : ranges)
    {
        range.TrySetByName(device, name, buffer, &bufferInfo, nullptr);
    }
}

void VulkanResourceBindingList::SetSampler(const char* name, SamplerState* sampler)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = sampler ? static_cast<VulkanSamplerState*>(sampler)->GetSampler() : VK_NULL_HANDLE;

    for (auto& range : ranges)
    {
        range.TrySetByName(device, name, sampler, nullptr, &imageInfo);
    }
}

void VulkanResourceBindingList::SetSRV(const char* name, ShaderResourceView* view)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = view ? static_cast<VulkanShaderResourceView*>(view)->GetImageView() : VK_NULL_HANDLE;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    for (auto& range : ranges)
    {
        range.TrySetByName(device, name, view, nullptr, &imageInfo);
    }
}

void VulkanResourceBindingList::SetUAV(const char* name, UnorderedAccessView* view, uint32_t initialCount)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = view ? static_cast<VulkanUnorderedAccessView*>(view)->GetImageView() : VK_NULL_HANDLE;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    for (auto& range : ranges)
    {
        range.TrySetByName(device, name, view, nullptr, &imageInfo);
    }
}

void VulkanResourceBindingList::SetCBV(const char* name, ShaderStage stage, Buffer* buffer)
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer ? static_cast<VulkanBuffer*>(buffer)->GetBuffer() : VK_NULL_HANDLE;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    ranges[StageIndex(stage)].TrySetByName(device, name, buffer, &bufferInfo, nullptr);
}

void VulkanResourceBindingList::SetSampler(const char* name, ShaderStage stage, SamplerState* sampler)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = sampler ? static_cast<VulkanSamplerState*>(sampler)->GetSampler() : VK_NULL_HANDLE;

    ranges[StageIndex(stage)].TrySetByName(device, name, sampler, nullptr, &imageInfo);
}

void VulkanResourceBindingList::SetSRV(const char* name, ShaderStage stage, ShaderResourceView* view)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = view ? static_cast<VulkanShaderResourceView*>(view)->GetImageView() : VK_NULL_HANDLE;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ranges[StageIndex(stage)].TrySetByName(device, name, view, nullptr, &imageInfo);
}

void VulkanResourceBindingList::SetUAV(const char* name, ShaderStage stage, UnorderedAccessView* view, uint32_t initialCount)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = view ? static_cast<VulkanUnorderedAccessView*>(view)->GetImageView() : VK_NULL_HANDLE;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    ranges[StageIndex(stage)].TrySetByName(device, name, view, nullptr, &imageInfo);
}

ResourceBindingList::iterator_pair VulkanResourceBindingList::GetSRVs()
{
    cachedSRVs.clear();
    for (auto& range : ranges)
    {
        range.CollectBindings(ShaderParameterType::SRV, cachedSRVs);
    }
    return { cachedSRVs.data(), cachedSRVs.data() + cachedSRVs.size() };
}

ResourceBindingList::iterator_pair VulkanResourceBindingList::GetCBVs()
{
    cachedCBVs.clear();
    for (auto& range : ranges)
    {
        range.CollectBindings(ShaderParameterType::CBV, cachedCBVs);
    }
    return { cachedCBVs.data(), cachedCBVs.data() + cachedCBVs.size() };
}

ResourceBindingList::iterator_pair VulkanResourceBindingList::GetUAVs()
{
    cachedUAVs.clear();
    for (auto& range : ranges)
    {
        range.CollectBindings(ShaderParameterType::UAV, cachedUAVs);
    }
    return { cachedUAVs.data(), cachedUAVs.data() + cachedUAVs.size() };
}

ResourceBindingList::iterator_pair VulkanResourceBindingList::GetSamplers()
{
    cachedSamplers.clear();
    for (auto& range : ranges)
    {
        range.CollectBindings(ShaderParameterType::Sampler, cachedSamplers);
    }
    return { cachedSamplers.data(), cachedSamplers.data() + cachedSamplers.size() };
}

HEXA_PRISM_NAMESPACE_END

#include "vulkan/resource_binding_list.hpp"
#include "vulkan/vulkan.hpp"
#include <spirv_reflect.h>
#include <map>

HEXA_PRISM_NAMESPACE_BEGIN

VulkanResourceBindingList::VulkanResourceBindingList(VulkanDevice* device, Pipeline* pipeline, const std::vector<VulkanReflectedStage>& stages)
    : device(device), pipeline(pipeline)
{
    struct Merged
    {
        std::string name;
        VkDescriptorType descriptorType;
        VkShaderStageFlags stageFlags = 0;
    };

    std::map<std::pair<uint32_t, uint32_t>, Merged> merged;

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

        for (auto* binding : spvBindings)
        {
            std::pair<uint32_t, uint32_t> key = { binding->set, binding->binding };
            auto it = merged.find(key);
            if (it == merged.end())
            {
                Merged entry;
                entry.name = binding->name ? binding->name : "";
                entry.descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type);
                entry.stageFlags = stageInfo.stage;
                merged.emplace(key, std::move(entry));
            }
            else
            {
                it->second.stageFlags |= stageInfo.stage;
            }
        }

        spvReflectDestroyShaderModule(&module);
    }

    uint32_t maxSet = 0;
    for (const auto& entry : merged)
    {
        maxSet = std::max(maxSet, entry.first.first);
    }

    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindingsPerSet(merged.empty() ? 0 : maxSet + 1);
    for (const auto& entry : merged)
    {
        uint32_t set = entry.first.first;
        uint32_t bindingIndex = entry.first.second;

        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = bindingIndex;
        layoutBinding.descriptorType = entry.second.descriptorType;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = entry.second.stageFlags;
        bindingsPerSet[set].push_back(layoutBinding);

        bindings.push_back({ entry.second.name, set, bindingIndex, entry.second.descriptorType });
    }

    VkDevice vkDevice = device->GetDevice();
    for (auto& setBindings : bindingsPerSet)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(setBindings.size());
        layoutInfo.pBindings = setBindings.data();

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

const VulkanResourceBindingList::BindingEntry* VulkanResourceBindingList::Find(const char* name) const
{
    if (!name)
    {
        return nullptr;
    }
    for (const auto& entry : bindings)
    {
        if (entry.name == name)
        {
            return &entry;
        }
    }
    return nullptr;
}

void VulkanResourceBindingList::WriteDescriptor(const BindingEntry& entry, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo)
{
    if (entry.set >= descriptorSets.size())
    {
        return;
    }

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSets[entry.set];
    write.dstBinding = entry.binding;
    write.descriptorCount = 1;
    write.descriptorType = entry.descriptorType;
    write.pBufferInfo = bufferInfo;
    write.pImageInfo = imageInfo;

    vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
}

void VulkanResourceBindingList::SetCBV(const char* name, Buffer* buffer)
{
    const BindingEntry* entry = Find(name);
    if (!entry || !buffer)
    {
        return;
    }

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = static_cast<VulkanBuffer*>(buffer)->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    WriteDescriptor(*entry, &bufferInfo, nullptr);
}

void VulkanResourceBindingList::SetSampler(const char* name, SamplerState* sampler)
{
    const BindingEntry* entry = Find(name);
    if (!entry || !sampler)
    {
        return;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = static_cast<VulkanSamplerState*>(sampler)->GetSampler();

    WriteDescriptor(*entry, nullptr, &imageInfo);
}

void VulkanResourceBindingList::SetSRV(const char* name, ShaderResourceView* view)
{
    const BindingEntry* entry = Find(name);
    if (!entry || !view)
    {
        return;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = static_cast<VulkanShaderResourceView*>(view)->GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    WriteDescriptor(*entry, nullptr, &imageInfo);
}

void VulkanResourceBindingList::SetUAV(const char* name, UnorderedAccessView* view, uint32_t initialCount)
{
    const BindingEntry* entry = Find(name);
    if (!entry || !view)
    {
        return;
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = static_cast<VulkanUnorderedAccessView*>(view)->GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    WriteDescriptor(*entry, nullptr, &imageInfo);
}

HEXA_PRISM_NAMESPACE_END

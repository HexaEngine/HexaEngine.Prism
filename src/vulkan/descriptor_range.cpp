#include "vulkan/descriptor_range.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/helper.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

    static VulkanShaderParameter* Find(VulkanShaderParameter* buckets, uint32_t capacity, uint32_t hash, const char* key)
    {
        uint32_t index = hash % capacity;
        bool exit = false;
        while (true)
        {
            auto entry = &buckets[index];
            if (entry->hash == 0)
            {
                return entry;
            }
            else if (entry->hash == hash && strcmp(key, entry->name.c_str()) == 0)
            {
                return entry;
            }

            index++;
            if (index == capacity)
            {
                if (exit)
                {
                    break;
                }

                index = 0;
                exit = true;
            }
        }

        return nullptr;
    }

    VulkanDescriptorRange::VulkanDescriptorRange(ShaderStage stage, VulkanShaderParameter* parameters, int parametersLength)
    {
        this->stage = stage;

        if (parametersLength > 0)
        {
            buckets = make_uarray_uninitialized<VulkanShaderParameter>(parametersLength);
            PrismZeroMemoryT(buckets.data(), parametersLength);

            for (int i = 0; i < parametersLength; i++)
            {
                VulkanShaderParameter& parameter = parameters[i];
                auto param = Find(buckets.data(), parametersLength, parameter.hash, parameter.name.c_str());
                *param = std::move(parameter);
            }
        }
    }

    VulkanDescriptorRange::VulkanDescriptorRange(VulkanDescriptorRange&& other) noexcept
        : stage(other.stage)
        , descriptorSet(other.descriptorSet)
        , buckets(std::move(other.buckets))
    {
        other.descriptorSet = VK_NULL_HANDLE;
    }

    VulkanDescriptorRange& VulkanDescriptorRange::operator=(VulkanDescriptorRange&& other) noexcept
    {
        if (this != &other)
        {
            stage = other.stage;
            descriptorSet = other.descriptorSet;
            buckets = std::move(other.buckets);
            other.descriptorSet = VK_NULL_HANDLE;
        }
        return *this;
    }

    VulkanShaderParameter* VulkanDescriptorRange::GetByName(const char* name) const
    {
        if (buckets.size() == 0)
        {
            throw std::out_of_range("Key not found");
        }

        uint32_t hash = HashString(name);
        auto pEntry = Find(buckets.data(), static_cast<uint32_t>(buckets.size()), hash, name);
        if (pEntry != nullptr && pEntry->hash == hash)
        {
            return pEntry;
        }
        throw std::out_of_range("Key not found");
    }

    bool VulkanDescriptorRange::TryGetByName(const char* name, VulkanShaderParameter*& parameter) const
    {
        if (buckets.size() == 0)
        {
            parameter = {};
            return false;
        }

        uint32_t hash = HashString(name);
        auto pEntry = Find(buckets.data(), static_cast<uint32_t>(buckets.size()), hash, name);
        if (pEntry != nullptr && pEntry->hash == hash)
        {
            parameter = pEntry;
            return true;
        }
        parameter = {};
        return false;
    }

    void VulkanDescriptorRange::SetByName(VulkanDevice* device, const char* name, void* resource, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo) const
    {
        auto parameter = GetByName(name);
        parameter->value = resource;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet;
        write.dstBinding = parameter->index;
        write.descriptorCount = 1;
        write.descriptorType = parameter->descriptorType;
        write.pBufferInfo = bufferInfo;
        write.pImageInfo = imageInfo;

        vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
    }

    bool VulkanDescriptorRange::TrySetByName(VulkanDevice* device, const char* name, void* resource, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo) const
    {
        VulkanShaderParameter* parameter;
        if (!TryGetByName(name, parameter))
        {
            return false;
        }
        parameter->value = resource;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet;
        write.dstBinding = parameter->index;
        write.descriptorCount = 1;
        write.descriptorType = parameter->descriptorType;
        write.pBufferInfo = bufferInfo;
        write.pImageInfo = imageInfo;

        vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
        return true;
    }

    bool VulkanDescriptorRange::TrySetUAVBySlot(VulkanDevice* device, uint32_t slot, void* resource, const VkDescriptorImageInfo* imageInfo) const
    {
        std::vector<VulkanShaderParameter*> uavParams;
        VulkanShaderParameter* data = buckets.data();
        for (size_t i = 0; i < buckets.size(); i++)
        {
            if (data[i].hash != 0 && data[i].type == ShaderParameterType::UAV)
            {
                uavParams.push_back(&data[i]);
            }
        }
        std::sort(uavParams.begin(), uavParams.end(), [](VulkanShaderParameter* a, VulkanShaderParameter* b) { return a->index < b->index; });

        if (slot >= uavParams.size())
        {
            return false;
        }

        VulkanShaderParameter* parameter = uavParams[slot];
        parameter->value = resource;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet;
        write.dstBinding = parameter->index;
        write.descriptorCount = 1;
        write.descriptorType = parameter->descriptorType;
        write.pImageInfo = imageInfo;

        vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
        return true;
    }

    std::vector<VkDescriptorSetLayoutBinding> VulkanDescriptorRange::BuildLayoutBindings() const
    {
        VkShaderStageFlagBits vkStage = ConvertStage(stage);

        std::vector<VkDescriptorSetLayoutBinding> result;
        result.reserve(buckets.size());
        for (size_t i = 0; i < buckets.size(); i++)
        {
            const VulkanShaderParameter& param = buckets[i];
            if (param.hash == 0)
            {
                continue;
            }

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = param.index;
            binding.descriptorType = param.descriptorType;
            binding.descriptorCount = 1;
            binding.stageFlags = vkStage;
            result.push_back(binding);
        }
        return result;
    }

    void VulkanDescriptorRange::CollectBindings(ShaderParameterType type, std::vector<BindingValuePair>& out) const
    {
        for (size_t i = 0; i < buckets.size(); i++)
        {
            const VulkanShaderParameter& param = buckets[i];
            if (param.type != type)
            {
                continue;
            }

            BindingValuePair pair;
            pair.name = param.name.c_str();
            pair.stage = param.stage;
            pair.type = param.type;
            pair.value = param.value;
            out.push_back(pair);
        }
    }

HEXA_PRISM_NAMESPACE_END

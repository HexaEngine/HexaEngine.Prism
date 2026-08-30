#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class VulkanDevice;

struct VulkanShaderParameter
{
    String name;
    uint32_t hash;
    uint32_t index;
    VkDescriptorType descriptorType;
    ShaderStage stage;
    ShaderParameterType type;
    void* value = nullptr;
};

struct VulkanDescriptorRange
{
    VulkanDescriptorRange() = default;
    VulkanDescriptorRange(ShaderStage stage, VulkanShaderParameter* parameters, int parametersLength);

    VulkanDescriptorRange(const VulkanDescriptorRange&) = delete;
    VulkanDescriptorRange& operator=(const VulkanDescriptorRange&) = delete;

    VulkanDescriptorRange(VulkanDescriptorRange&& other) noexcept;
    VulkanDescriptorRange& operator=(VulkanDescriptorRange&& other) noexcept;

    ShaderStage stage = ShaderStage::Vertex;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    uarray<VulkanShaderParameter> buckets;

    static uint32_t HashString(const char* str)
    {
        uint32_t hash = 0x811c9dc5u;
        while (*str)
        {
            hash ^= static_cast<uint32_t>(*str++);
            hash *= 0x01000193u;
        }
        return hash;
    }

    VulkanShaderParameter* GetByName(const char* name) const;

    bool TryGetByName(const char* name, VulkanShaderParameter*& parameter) const;

    void SetByName(VulkanDevice* device, const char* name, void* resource, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo) const;

    bool TrySetByName(VulkanDevice* device, const char* name, void* resource, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo) const;

    // Resolves a UAV by its D3D-style register slot (the Nth UAV declared in the shader, in
    // ascending binding order) rather than by name - for OMSetRenderTargetsAndUnorderedAccessViews,
    // which is inherently slot-based, unlike the rest of this binding-by-name model.
    bool TrySetUAVBySlot(VulkanDevice* device, uint32_t slot, void* resource, const VkDescriptorImageInfo* imageInfo) const;

    std::vector<VkDescriptorSetLayoutBinding> BuildLayoutBindings() const;

    void CollectBindings(ShaderParameterType type, std::vector<BindingValuePair>& out) const;
};

HEXA_PRISM_NAMESPACE_END

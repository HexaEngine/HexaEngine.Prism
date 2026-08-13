#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class VulkanDevice;

struct VulkanReflectedStage
{
	const uint32_t* code;
	size_t codeSize;
	VkShaderStageFlagBits stage;
};

// Reflects the compiled SPIR-V for every stage of a pipeline (via SPIRV-Reflect) to resolve
// name-based bindings (SetCBV("name", ...) etc.) to actual descriptor sets, mirroring what
// D3D11ResourceBindingList does via D3DReflect.
class VulkanResourceBindingList final : public ResourceBindingList
{
	struct BindingEntry
	{
		std::string name;
		uint32_t set;
		uint32_t binding;
		VkDescriptorType descriptorType;
	};

	VulkanDevice* device;
	Pipeline* pipeline;
	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<BindingEntry> bindings;

	const BindingEntry* Find(const char* name) const;
	void WriteDescriptor(const BindingEntry& entry, const VkDescriptorBufferInfo* bufferInfo, const VkDescriptorImageInfo* imageInfo);

public:
	VulkanResourceBindingList(VulkanDevice* device, Pipeline* pipeline, const std::vector<VulkanReflectedStage>& stages);
	~VulkanResourceBindingList() override;

	Pipeline* GetPipeline() const override { return pipeline; }

	void SetCBV(const char* name, Buffer* buffer) override;
	void SetSampler(const char* name, SamplerState* sampler) override;
	void SetSRV(const char* name, ShaderResourceView* view) override;
	void SetUAV(const char* name, UnorderedAccessView* view, uint32_t initialCount = static_cast<uint32_t>(-1)) override;

	void SetCBV(const char* name, ShaderStage stage, Buffer* buffer) override { SetCBV(name, buffer); }
	void SetSampler(const char* name, ShaderStage stage, SamplerState* sampler) override { SetSampler(name, sampler); }
	void SetSRV(const char* name, ShaderStage stage, ShaderResourceView* view) override { SetSRV(name, view); }
	void SetUAV(const char* name, ShaderStage stage, UnorderedAccessView* view, uint32_t initialCount = static_cast<uint32_t>(-1)) override { SetUAV(name, view, initialCount); }

	// TODO: Implement iterators (matches D3D11ResourceBindingList's own current gap).
	iterator_pair GetSRVs() override { return {}; }
	iterator_pair GetCBVs() override { return {}; }
	iterator_pair GetUAVs() override { return {}; }
	iterator_pair GetSamplers() override { return {}; }

	const std::vector<VkDescriptorSetLayout>& GetSetLayouts() const noexcept { return setLayouts; }
	const std::vector<VkDescriptorSet>& GetDescriptorSets() const noexcept { return descriptorSets; }
};

HEXA_PRISM_NAMESPACE_END

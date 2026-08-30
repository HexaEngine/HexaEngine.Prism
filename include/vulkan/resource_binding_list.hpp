#pragma once
#include "common.hpp"
#include "descriptor_range.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class VulkanDevice;

struct VulkanReflectedStage
{
	const uint32_t* code;
	size_t codeSize;
	VkShaderStageFlagBits stage;
};

class VulkanResourceBindingList final : public ResourceBindingList
{
	static constexpr size_t StageCount = 5;

	VulkanDevice* device;
	Pipeline* pipeline;
	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkDescriptorSet> descriptorSets;
	std::array<VulkanDescriptorRange, StageCount> ranges;
	std::vector<BindingValuePair> cachedSRVs;
	std::vector<BindingValuePair> cachedCBVs;
	std::vector<BindingValuePair> cachedUAVs;
	std::vector<BindingValuePair> cachedSamplers;

	static size_t StageIndex(ShaderStage stage) { return stage == ShaderStage::Compute ? 0 : static_cast<size_t>(stage); }

public:
	VulkanResourceBindingList(VulkanDevice* device, Pipeline* pipeline, const std::vector<VulkanReflectedStage>& stages);
	~VulkanResourceBindingList() override;

	Pipeline* GetPipeline() const override { return pipeline; }

	void SetCBV(const char* name, Buffer* buffer) override;
	void SetSampler(const char* name, SamplerState* sampler) override;
	void SetSRV(const char* name, ShaderResourceView* view) override;
	void SetUAV(const char* name, UnorderedAccessView* view, uint32_t initialCount = static_cast<uint32_t>(-1)) override;

	void SetCBV(const char* name, ShaderStage stage, Buffer* buffer) override;
	void SetSampler(const char* name, ShaderStage stage, SamplerState* sampler) override;
	void SetSRV(const char* name, ShaderStage stage, ShaderResourceView* view) override;
	void SetUAV(const char* name, ShaderStage stage, UnorderedAccessView* view, uint32_t initialCount = static_cast<uint32_t>(-1)) override;

	iterator_pair GetSRVs() override;
	iterator_pair GetCBVs() override;
	iterator_pair GetUAVs() override;
	iterator_pair GetSamplers() override;

	bool TrySetPixelUAVBySlot(VulkanDevice* boundDevice, uint32_t slot, void* resource, const VkDescriptorImageInfo* imageInfo)
	{
		return ranges[StageIndex(ShaderStage::Pixel)].TrySetUAVBySlot(boundDevice, slot, resource, imageInfo);
	}

	const std::vector<VkDescriptorSetLayout>& GetSetLayouts() const noexcept { return setLayouts; }
	const std::vector<VkDescriptorSet>& GetDescriptorSets() const noexcept { return descriptorSets; }
};

HEXA_PRISM_NAMESPACE_END

#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class D3D11ComputePipeline final : public ComputePipeline
{
	friend class D3D11ResourceBindingList;
	friend class D3D11ComputePipelineState;
	D3D11GraphicsDevice* device;
	ComPtr<ID3D11ComputeShader> cs;

	PrismObj<Blob> computeShaderBlob;
	bool valid;

public:
	D3D11ComputePipeline(D3D11GraphicsDevice* device, const ComputePipelineDesc& desc);

	void Compile();
};

HEXA_PRISM_NAMESPACE_END
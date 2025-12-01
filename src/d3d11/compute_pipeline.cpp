#include "d3d11/compute_pipeline.hpp"
#include "d3d11/d3d11.hpp"
#include "helpers.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

D3D11ComputePipeline::D3D11ComputePipeline(D3D11GraphicsDevice* device, const ComputePipelineDesc& desc) : ComputePipeline(desc), device(device), valid(false)
{
	Compile();
}

void D3D11ComputePipeline::Compile()
{
	auto dev = device->GetDevice();
	bool success = true;

	success &= CompileAndCreateShader(
		dev, desc.computeShader, desc.computeEntryPoint, "cs_5_0",
		computeShaderBlob, cs,
		&ID3D11Device::CreateComputeShader
	);

	valid = success;
}

HEXA_PRISM_NAMESPACE_END
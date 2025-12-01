#include "d3d11/compute_pipeline_state.hpp"
#include "d3d11/d3d11.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

D3D11ComputePipelineState::D3D11ComputePipelineState(const PrismObj<D3D11ComputePipeline>& pipeline, const ComputePipelineStateDesc& desc)
	: ComputePipelineState(pipeline, desc), isValid(false)
{
    bindingList = std::make_unique<D3D11ResourceBindingList>(pipeline.Get(), desc.flags);
	isValid = true;
}

void D3D11ComputePipelineState::SetState(ID3D11DeviceContext3* context)
{
    auto pipe = pipeline.AsPtr<D3D11ComputePipeline>();
    context->CSSetShader(pipe->cs.Get(), nullptr, 0);

    bindingList->BindCompute(context);
}

void D3D11ComputePipelineState::UnsetState(ID3D11DeviceContext3* context)
{
    context->CSSetShader(nullptr, nullptr, 0);

    bindingList->UnbindCompute(context);
}


HEXA_PRISM_NAMESPACE_END


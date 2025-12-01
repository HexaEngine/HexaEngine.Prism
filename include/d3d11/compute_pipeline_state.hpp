#pragma once
#include "common.hpp"
#include "compute_pipeline.hpp"
#include "resource_binding_list.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class D3D11ComputePipelineState : public ComputePipelineState
{
	std::unique_ptr<D3D11ResourceBindingList> bindingList;
	bool isValid;

public:
	D3D11ComputePipelineState(const PrismObj<D3D11ComputePipeline>& pipeline, const ComputePipelineStateDesc& desc);
	ResourceBindingList& GetBindings() override { return *bindingList.get(); }

    void SetState(ID3D11DeviceContext3* context);
    void UnsetState(ID3D11DeviceContext3* context);
};

HEXA_PRISM_NAMESPACE_END
#pragma once
#include "common.hpp"
#include "graphics_pipeline.hpp"
#include "resource_binding_list.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class D3D11GraphicsPipelineState : public GraphicsPipelineState
{
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11RasterizerState2> rasterizerState;
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	ComPtr<ID3D11BlendState1> blendState;
	std::unique_ptr<D3D11ResourceBindingList> bindingList;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
	bool isValid;

	void CreateLayout(const InputElementDescription* inputElements, size_t numInputElements, Blob* signature);
public:
	D3D11GraphicsPipelineState(const PrismObj<D3D11GraphicsPipeline>& pipeline, const GraphicsPipelineStateDesc& desc);

	ResourceBindingList& GetBindings() override { return *bindingList.get(); }

	void SetState(ID3D11DeviceContext3* context);
	void UnsetState(ID3D11DeviceContext3* context);
};

HEXA_PRISM_NAMESPACE_END
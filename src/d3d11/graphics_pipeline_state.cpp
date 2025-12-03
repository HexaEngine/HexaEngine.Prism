#include "d3d11/graphics_pipeline_state.hpp"
#include "d3d11/d3d11.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
	D3D11_RASTERIZER_DESC2 ConvertRasterizerDesc(const RasterizerDescription& desc)
	{
		D3D11_RASTERIZER_DESC2 result = {};
		result.FillMode = static_cast<D3D11_FILL_MODE>(desc.fillMode);
		result.CullMode = static_cast<D3D11_CULL_MODE>(desc.cullMode);
		result.FrontCounterClockwise = desc.frontCounterClockwise ? TRUE : FALSE;
		result.DepthBias = desc.depthBias;
		result.DepthBiasClamp = desc.depthBiasClamp;
		result.SlopeScaledDepthBias = desc.slopeScaledDepthBias;
		result.DepthClipEnable = desc.depthClipEnable ? TRUE : FALSE;
		result.ScissorEnable = desc.scissorEnable ? TRUE : FALSE;
		result.MultisampleEnable = desc.multisampleEnable ? TRUE : FALSE;
		result.AntialiasedLineEnable = desc.antialiasedLineEnable ? TRUE : FALSE;
		result.ForcedSampleCount = desc.forcedSampleCount;
		result.ConservativeRaster = static_cast<D3D11_CONSERVATIVE_RASTERIZATION_MODE>(desc.conservativeRaster);
		return result;
	}

	D3D11_DEPTH_STENCIL_DESC ConvertDepthStencilDesc(const DepthStencilDescription& desc)
	{
		D3D11_DEPTH_STENCIL_DESC result = {};
		result.DepthEnable = desc.depthEnable ? TRUE : FALSE;
		result.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(desc.depthWriteMask);
		result.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(desc.depthFunc);
		result.StencilEnable = desc.stencilEnable ? TRUE : FALSE;
		result.StencilReadMask = desc.stencilReadMask;
		result.StencilWriteMask = desc.stencilWriteMask;
		
		result.FrontFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(desc.frontFace.stencilFailOp);
		result.FrontFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(desc.frontFace.stencilDepthFailOp);
		result.FrontFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(desc.frontFace.stencilPassOp);
		result.FrontFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(desc.frontFace.stencilFunc);
		
		result.BackFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(desc.backFace.stencilFailOp);
		result.BackFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(desc.backFace.stencilDepthFailOp);
		result.BackFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(desc.backFace.stencilPassOp);
		result.BackFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(desc.backFace.stencilFunc);
		
		return result;
	}

	D3D11_BLEND_DESC1 ConvertBlendDesc(const BlendDescription& desc)
	{
		D3D11_BLEND_DESC1 result = {};
		result.AlphaToCoverageEnable = desc.alphaToCoverageEnable ? TRUE : FALSE;
		result.IndependentBlendEnable = desc.independentBlendEnable ? TRUE : FALSE;
		
		for (size_t i = 0; i < BlendDescription::SimultaneousRenderTargetCount; i++)
		{
			const auto& src = desc.renderTargets[i];
			auto& dst = result.RenderTarget[i];
			
			dst.BlendEnable = src.isBlendEnabled ? TRUE : FALSE;
			dst.LogicOpEnable = src.isLogicOpEnabled ? TRUE : FALSE;
			dst.SrcBlend = static_cast<D3D11_BLEND>(src.sourceBlend);
			dst.DestBlend = static_cast<D3D11_BLEND>(src.destinationBlend);
			dst.BlendOp = static_cast<D3D11_BLEND_OP>(src.blendOp);
			dst.SrcBlendAlpha = static_cast<D3D11_BLEND>(src.sourceBlendAlpha);
			dst.DestBlendAlpha = static_cast<D3D11_BLEND>(src.destinationBlendAlpha);
			dst.BlendOpAlpha = static_cast<D3D11_BLEND_OP>(src.blendOpAlpha);
			dst.LogicOp = static_cast<D3D11_LOGIC_OP>(src.logicOp);
			dst.RenderTargetWriteMask = static_cast<UINT8>(src.renderTargetWriteMask);
		}
		
		return result;
	}

	D3D_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(PrimitiveTopology topology)
	{
		return static_cast<D3D_PRIMITIVE_TOPOLOGY>(topology);
	}

	D3D11_INPUT_ELEMENT_DESC ConvertInputElement(const InputElementDescription& elem)
	{
		D3D11_INPUT_ELEMENT_DESC result = {};
		result.SemanticName = elem.semanticName.c_str();
		result.SemanticIndex = elem.semanticIndex;
		result.Format = static_cast<DXGI_FORMAT>(elem.format);
		result.InputSlot = elem.slot;
		result.AlignedByteOffset = elem.alignedByteOffset;
		result.InputSlotClass = static_cast<D3D11_INPUT_CLASSIFICATION>(elem.classification);
		result.InstanceDataStepRate = elem.instanceDataStepRate;
		return result;
	}
}

static bool CanSkipLayout(const InputElementDescription* inputElements, size_t numInputElements)
{
	if (!inputElements || numInputElements == 0)
		return true;

	for (size_t i = 0; i < numInputElements; i++)
	{
		auto& semanticName = inputElements[i].semanticName;
		if (semanticName != "SV_VertexID" && semanticName != "SV_InstanceID")
		{
			return false;
		}
	}

	return true;
}

void D3D11GraphicsPipelineState::CreateLayout(const InputElementDescription* inputElements, size_t numInputElements, Blob* signature)
{
	auto device = pipeline.AsPtr<D3D11GraphicsPipeline>()->device;
	isValid = false;
	
	if (inputLayout)
	{
		inputLayout.Reset();
	}

	if (!inputElements || numInputElements == 0)
	{
		return;
	}

	if (!CanSkipLayout(inputElements, numInputElements))
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> d3dInputElements(numInputElements);
		for (size_t i = 0; i < numInputElements; i++)
		{
			d3dInputElements[i] = ConvertInputElement(inputElements[i]);
		}

		HRESULT hr = device->GetDevice()->CreateInputLayout(
			d3dInputElements.data(),
			static_cast<UINT>(numInputElements),
			signature->GetData(),
			signature->GetLength(),
			&inputLayout
		);

		if (FAILED(hr))
		{
			return;
		}
	}

	isValid = true;
}

D3D11GraphicsPipelineState::D3D11GraphicsPipelineState(const PrismObj<D3D11GraphicsPipeline>& pipeline, const GraphicsPipelineStateDesc& desc)
	: GraphicsPipelineState(pipeline, desc),
	  bindingList(nullptr),
	  primitiveTopology(ConvertPrimitiveTopology(desc.primitiveTopology)),
	  isValid(false)
{
	auto device = pipeline.AsPtr<D3D11GraphicsPipeline>()->device;

	if (pipeline->vertexShaderBlob)
	{
		const InputElementDescription* inputElements = desc.inputElements;
		size_t numInputElements = desc.numInputElements;

		if (!inputElements || numInputElements == 0)
		{
			inputElements = pipeline->inputElements.data();
			numInputElements = pipeline->inputElements.size();
		}
		
		CreateLayout(inputElements, numInputElements, pipeline->signatureBlob.Get());
		isValid = true;
	}

	{
		D3D11_RASTERIZER_DESC2 rsDesc = ConvertRasterizerDesc(desc.rasterizer);
		HRESULT hr = device->GetDevice()->CreateRasterizerState2(&rsDesc, &rasterizerState);
		if (FAILED(hr))
		{
			isValid = false;
		}
	}

	{
		D3D11_DEPTH_STENCIL_DESC dsDesc = ConvertDepthStencilDesc(desc.depthStencil);
		HRESULT hr = device->GetDevice()->CreateDepthStencilState(&dsDesc, &depthStencilState);
		if (FAILED(hr))
		{
			isValid = false;
		}
	}

	{
		D3D11_BLEND_DESC1 bsDesc = ConvertBlendDesc(desc.blend);
		HRESULT hr = device->GetDevice()->CreateBlendState1(&bsDesc, &blendState);
		if (FAILED(hr))
		{
			isValid = false;
		}
	}

	bindingList = std::make_unique<D3D11ResourceBindingList>(pipeline.Get(), desc.flags);
}

void D3D11GraphicsPipelineState::SetState(ID3D11DeviceContext3* context)
{
	auto pipe = pipeline.AsPtr<D3D11GraphicsPipeline>();
	context->VSSetShader(pipe->vs.Get(), nullptr, 0);
	context->HSSetShader(pipe->hs.Get(), nullptr, 0);
	context->DSSetShader(pipe->ds.Get(), nullptr, 0);
	context->GSSetShader(pipe->gs.Get(), nullptr, 0);
	context->PSSetShader(pipe->ps.Get(), nullptr, 0);

	context->RSSetState(rasterizerState.Get());

	float blendFactor[4] = { desc.blendFactor.r, desc.blendFactor.g, desc.blendFactor.b, desc.blendFactor.a };
	context->OMSetBlendState(blendState.Get(), blendFactor, desc.sampleMask);
	context->OMSetDepthStencilState(depthStencilState.Get(), desc.stencilRef);
	context->IASetInputLayout(inputLayout.Get());
	context->IASetPrimitiveTopology(primitiveTopology);

	bindingList->BindGraphics(context);
}

void D3D11GraphicsPipelineState::UnsetState(ID3D11DeviceContext3* context)
{
	context->VSSetShader(nullptr, nullptr, 0);
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);

	context->RSSetState(nullptr);
	context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(nullptr, 0);
	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);

	bindingList->UnbindGraphics(context);
}

HEXA_PRISM_NAMESPACE_END
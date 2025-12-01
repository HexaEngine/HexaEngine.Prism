#include "d3d11/d3d11.hpp"
#include "d3d11/graphics_pipeline.hpp"
#include "helpers.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
	Format GetFormatFromSignature(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask)
	{
		if (mask == (D3D_COMPONENT_MASK_X))
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return Format::R32UInt;
			case D3D_REGISTER_COMPONENT_SINT32:
				return Format::R32SInt;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				return Format::R32Float;
			default:
				return Format::Unknown;
			}
		}

		if (mask == (D3D_COMPONENT_MASK_X | D3D_COMPONENT_MASK_Y))
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return Format::R32G32UInt;
			case D3D_REGISTER_COMPONENT_SINT32:
				return Format::R32G32SInt;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				return Format::R32G32Float;
			default:
				return Format::Unknown;
			}
		}

		if (mask == (D3D_COMPONENT_MASK_X | D3D_COMPONENT_MASK_Y | D3D_COMPONENT_MASK_Z))
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return Format::R32G32B32UInt;
			case D3D_REGISTER_COMPONENT_SINT32:
				return Format::R32G32B32SInt;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				return Format::R32G32B32Float;
			default:
				return Format::Unknown;
			}
		}

		if (mask == (D3D_COMPONENT_MASK_X | D3D_COMPONENT_MASK_Y | D3D_COMPONENT_MASK_Z | D3D_COMPONENT_MASK_W))
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return Format::R32G32B32A32UInt;
			case D3D_REGISTER_COMPONENT_SINT32:
				return Format::R32G32B32A32SInt;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				return Format::R32G32B32A32Float;
			default:
				return Format::Unknown;
			}
		}

		return Format::Unknown;
	}

	void GetInputElementsFromSignature(const PrismObj<Blob>& shaderBlob, std::vector<InputElementDescription>& inputElements)
	{
		inputElements.clear();
		if (!shaderBlob)
		{
			return;
		}

		ComPtr<ID3D11ShaderReflection> reflection;
		HRESULT hr = D3DReflect(shaderBlob->GetData(), shaderBlob->GetLength(), IID_PPV_ARGS(&reflection));
		if (FAILED(hr))
		{
			return;
		}

		D3D11_SHADER_DESC shaderDesc;
		hr = reflection->GetDesc(&shaderDesc);
		if (FAILED(hr))
		{
			return;
		}

		inputElements.reserve(shaderDesc.InputParameters);

		for (UINT i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC parameterDesc;
			hr = reflection->GetInputParameterDesc(i, &parameterDesc);
			if (FAILED(hr))
			{
				continue;
			}

			InputElementDescription inputElement = {};
			inputElement.semanticName = parameterDesc.SemanticName;
			inputElement.semanticIndex = parameterDesc.SemanticIndex;
			inputElement.slot = 0;
			inputElement.alignedByteOffset = InputElementDescription::AppendAligned;
			inputElement.classification = InputClassification::PerVertexData;
			inputElement.instanceDataStepRate = 0;
			inputElement.format = GetFormatFromSignature(parameterDesc.ComponentType, parameterDesc.Mask);

			inputElements.push_back(inputElement);
		}
	}
}

D3D11GraphicsPipeline::D3D11GraphicsPipeline(D3D11GraphicsDevice* device, const GraphicsPipelineDesc& desc) : GraphicsPipeline(desc), device(device), valid(false)
{
	Compile();
}

void D3D11GraphicsPipeline::Compile()
{
	auto dev = device->GetDevice();
	bool success = true;

	success &= CompileAndCreateShader(
		dev, desc.vertexShader, desc.vertexEntryPoint, "vs_5_0",
		vertexShaderBlob, vs,
		&ID3D11Device::CreateVertexShader
	);

	success &= CompileAndCreateShader(
		dev, desc.hullShader, desc.hullEntryPoint, "hs_5_0",
		hullShaderBlob, hs,
		&ID3D11Device::CreateHullShader
	);

	success &= CompileAndCreateShader(
		dev, desc.domainShader, desc.domainEntryPoint, "ds_5_0",
		domainShaderBlob, ds,
		&ID3D11Device::CreateDomainShader
	);

	success &= CompileAndCreateShader(
		dev, desc.geometryShader, desc.geometryEntryPoint, "gs_5_0",
		geometryShaderBlob, gs,
		&ID3D11Device::CreateGeometryShader
	);

	success &= CompileAndCreateShader(
		dev, desc.pixelShader, desc.pixelEntryPoint, "ps_5_0",
		pixelShaderBlob, ps,
		&ID3D11Device::CreatePixelShader
	);

	if (vertexShaderBlob)
	{
		ComPtr<ID3D10Blob> signature;
		D3DGetInputSignatureBlob(vertexShaderBlob->GetData(), vertexShaderBlob->GetLength(), signature.GetAddressOf());
		if (signature)
		{
			auto sigSize = signature->GetBufferSize();
			auto sigData = static_cast<uint8_t*>(signature->GetBufferPointer());
			signatureBlob = MakePrismObj<Blob>(sigData, sigSize, false, true);
		}

		GetInputElementsFromSignature(vertexShaderBlob, inputElements);
	}

	valid = success;
}

HEXA_PRISM_NAMESPACE_END
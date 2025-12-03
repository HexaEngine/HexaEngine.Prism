#pragma once
#include "common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

class D3D11GraphicsPipeline final : public GraphicsPipeline
{
	friend class D3D11ResourceBindingList;
	friend class D3D11GraphicsPipelineState;
protected:
	D3D11GraphicsDevice* device;
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11HullShader> hs;
	ComPtr<ID3D11DomainShader> ds;
	ComPtr<ID3D11GeometryShader> gs;
	ComPtr<ID3D11PixelShader> ps;

	PrismObj<Blob> vertexShaderBlob;
	PrismObj<Blob> hullShaderBlob;
	PrismObj<Blob> domainShaderBlob;
	PrismObj<Blob> geometryShaderBlob;
	PrismObj<Blob> pixelShaderBlob;
	PrismObj<Blob> signatureBlob;
	std::vector<InputElementDescription> inputElements;
	bool valid;

public:
	D3D11GraphicsPipeline(D3D11GraphicsDevice* device, const GraphicsPipelineDesc& desc);
	~D3D11GraphicsPipeline() override = default;

	void Compile();
	bool IsValid() const noexcept { return valid; }
};

HEXA_PRISM_NAMESPACE_END
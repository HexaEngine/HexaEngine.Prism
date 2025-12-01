#pragma once
#include "d3d11/d3d11.hpp"
#include "d3d11/shader_compiler.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
	template<typename TShaderInterface>
	bool CompileAndCreateShader(
		ID3D11Device* device,
		ShaderSource* source,
		const char* entryPoint,
		const char* targetProfile,
		PrismObj<Blob>& blobOut,
		ComPtr<TShaderInterface>& shaderOut,
		HRESULT(ID3D11Device::* createFunc)(const void*, SIZE_T, ID3D11ClassLinkage*, TShaderInterface**)
	)
	{
		if (!source)
			return true;

		bool ok = D3D11ShaderCompiler::Compile(source, entryPoint, targetProfile, blobOut);
		if (!ok)
			return false;

		HRESULT hr = (device->*createFunc)(
			blobOut->GetData(),
			blobOut->GetLength(),
			nullptr,
			shaderOut.GetAddressOf()
			);

		return SUCCEEDED(hr);
	}
}

HEXA_PRISM_NAMESPACE_END
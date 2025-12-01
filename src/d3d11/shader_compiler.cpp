#include "d3d11/shader_compiler.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

	bool D3D11ShaderCompiler::Compile(ShaderSource* source, const char* entryPoint, const char* profile, PrismObj<Blob>& shaderOut)
	{
		if (!entryPoint)
		{
			entryPoint = "main";
		}

		uint8_t* ptr;
		size_t len;
		source->GetData(ptr, len);
		auto name = source->GetIdentifier();
		ComPtr<ID3D10Blob> codeBlob;
		ComPtr<ID3D10Blob> errorBlob;
		auto hr = D3DCompile(ptr, len, name, nullptr, nullptr, entryPoint, profile, 0, 0, codeBlob.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob)
		{
			auto errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
			size_t errorLen = errorBlob->GetBufferSize();
			std::string errorStr(errorMsg, errorLen);

			// TODO: Add Logging.
			// PrismLogError("Shader compilation error (%s): %s", name, errorStr.c_str());
			std::cout << "Shader compilation error (" << name << "): " << errorStr << std::endl;
		}

		if (codeBlob)
		{
			auto buffer = static_cast<uint8_t*>(codeBlob->GetBufferPointer());
			auto bufferSize = codeBlob->GetBufferSize();
			uint8_t* bytecode = PrismAllocT<uint8_t>(bufferSize);
			PrismMemoryCopyT(bytecode, buffer, bufferSize);
			shaderOut = MakePrismObj<Blob>(bytecode, bufferSize, true);
		}

		return SUCCEEDED(hr);
	}

HEXA_PRISM_NAMESPACE_END

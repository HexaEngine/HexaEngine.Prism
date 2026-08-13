#include "vulkan/shader_compiler.hpp"

#if HEXA_PRISM_WINDOWS
#include <windows.h>
#endif
#include <dxc/dxcapi.h>

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
    const wchar_t* ConvertTargetProfile(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex: return L"vs_6_0";
        case ShaderStage::Hull: return L"hs_6_0";
        case ShaderStage::Domain: return L"ds_6_0";
        case ShaderStage::Geometry: return L"gs_6_0";
        case ShaderStage::Pixel: return L"ps_6_0";
        case ShaderStage::Compute: return L"cs_6_0";
        default: return L"lib_6_0";
        }
    }

    std::wstring ToWide(const char* str)
    {
        return std::wstring(str, str + std::strlen(str));
    }
}

bool VulkanShaderCompiler::Compile(ShaderSource* source, const char* entryPoint, ShaderStage stage, PrismObj<Blob>& shaderOut)
{
    if (!entryPoint)
    {
        entryPoint = "main";
    }

    uint8_t* ptr;
    size_t len;
    source->GetData(ptr, len);
    const char* name = source->GetIdentifier();

    IDxcUtils* utils = nullptr;
    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))))
    {
        return false;
    }

    IDxcCompiler3* compiler = nullptr;
    if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
    {
        utils->Release();
        return false;
    }

    IDxcBlobEncoding* sourceBlob = nullptr;
    utils->CreateBlob(ptr, static_cast<uint32_t>(len), DXC_CP_UTF8, &sourceBlob);

    DxcBuffer sourceBuffer = {};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = DXC_CP_UTF8;

    std::wstring wEntryPoint = ToWide(entryPoint);
    const wchar_t* targetProfile = ConvertTargetProfile(stage);

    LPCWSTR args[] =
    {
        L"-E", wEntryPoint.c_str(),
        L"-T", targetProfile,
        L"-spirv",
        L"-fspv-target-env=vulkan1.3",
    };

    IDxcResult* result = nullptr;
    compiler->Compile(&sourceBuffer, args, static_cast<uint32_t>(std::size(args)), nullptr, IID_PPV_ARGS(&result));

    bool success = false;
    if (result)
    {
        IDxcBlobUtf8* errors = nullptr;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetStringLength() > 0)
        {
            std::cout << "Shader compilation error (" << name << "): " << errors->GetStringPointer() << std::endl;
        }
        if (errors)
        {
            errors->Release();
        }

        HRESULT status = E_FAIL;
        result->GetStatus(&status);
        success = SUCCEEDED(status);

        if (success)
        {
            IDxcBlob* codeBlob = nullptr;
            result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&codeBlob), nullptr);
            if (codeBlob)
            {
                size_t bufferSize = codeBlob->GetBufferSize();
                uint8_t* bytecode = PrismAllocT<uint8_t>(bufferSize);
                PrismMemoryCopy(bytecode, codeBlob->GetBufferPointer(), bufferSize);
                shaderOut = MakePrismObj<Blob>(bytecode, bufferSize, true);
                codeBlob->Release();
            }
            else
            {
                success = false;
            }
        }

        result->Release();
    }

    sourceBlob->Release();
    compiler->Release();
    utils->Release();

    return success;
}

HEXA_PRISM_NAMESPACE_END

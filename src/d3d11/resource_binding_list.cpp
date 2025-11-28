#include "resource_binding_list.hpp"
#include "d3d11.hpp"
#include <d3dcompiler.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>

HEXA_PRISM_NAMESPACE_BEGIN

D3D11GlobalResourceList::StateChangedHandlerList D3D11GlobalResourceList::StateChanged = {};

D3D11ResourceBindingList::D3D11ResourceBindingList(D3D11GraphicsPipeline* pipeline, PipelineStateFlags flags)
    : pipeline(pipeline), flags(flags)
{
    pipeline->AddRef();
    // onCompileToken = pipeline->OnCompile.Subscribe([this](Pipeline* p) { OnPipelineCompile(p); });
    OnPipelineCompile(pipeline);
}

D3D11ResourceBindingList::D3D11ResourceBindingList(D3D11ComputePipeline* pipeline, PipelineStateFlags flags)
    : pipeline(pipeline), flags(flags)
{
    pipeline->AddRef();
    // onCompileToken = pipeline->OnCompile.Subscribe([this](Pipeline* p) { OnPipelineCompile(p); });
    OnPipelineCompile(pipeline);
}

D3D11ResourceBindingList::~D3D11ResourceBindingList()
{
    globalStateChangedToken.Unsubscribe();
    onCompileToken.Unsubscribe();
    
    pipeline->Release();

    Clear();
}

void D3D11ResourceBindingList::Hook()
{
    globalStateChangedToken = D3D11GlobalResourceList::StateChanged.Subscribe(
        [this](const char* name, D3D11ShaderParameterState oldState, D3D11ShaderParameterState state)
        {
            GlobalStateChanged(name, oldState, state);
        });
}

void D3D11ResourceBindingList::GlobalStateChanged(const char* name, D3D11ShaderParameterState oldState, D3D11ShaderParameterState state)
{
    switch (state.Type)
    {
    case ShaderParameterType::SRV:
        for (auto& range : rangesSRVs)
        {
            range.UpdateByName(name, oldState.Resource, state.Resource);
        }
        break;

    case ShaderParameterType::UAV:
        for (auto& range : rangesUAVs)
        {
            range.UpdateByName(name, oldState.Resource, state.Resource, state.InitialCount);
        }
        break;

    case ShaderParameterType::CBV:
        for (auto& range : rangesCBVs)
        {
            range.UpdateByName(name, oldState.Resource, state.Resource);
        }
        break;

    case ShaderParameterType::Sampler:
        for (auto& range : rangesSamplers)
        {
            range.UpdateByName(name, oldState.Resource, state.Resource);
        }
        break;
    }
}

void D3D11ResourceBindingList::OnPipelineCompile(Pipeline* pipeline)
{
    Clear();
    
    // TODO: Implement shader reflection and binding setup
    // This requires access to shader blob data and D3D11 shader reflection API
    
    D3D11GlobalResourceList::SetState(this);
}

D3D11GraphicsDevice* D3D11ResourceBindingList::GetDevice()
{
    // TODO: Implement device retrieval from pipeline
    return nullptr;
}

static ShaderParameterType ConvertShaderInputType(D3D_SHADER_INPUT_TYPE type)
{
    switch (type)
    {
    case D3D_SIT_CBUFFER:
        return ShaderParameterType::CBV;
    case D3D_SIT_TBUFFER:
    case D3D_SIT_TEXTURE:
    case D3D_SIT_STRUCTURED:
    case D3D_SIT_BYTEADDRESS:
        return ShaderParameterType::SRV;
    case D3D_SIT_SAMPLER:
        return ShaderParameterType::Sampler;
    case D3D_SIT_UAV_RWTYPED:
    case D3D_SIT_UAV_RWSTRUCTURED:
    case D3D_SIT_UAV_RWBYTEADDRESS:
    case D3D_SIT_UAV_APPEND_STRUCTURED:
    case D3D_SIT_UAV_CONSUME_STRUCTURED:
    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
    case D3D_SIT_UAV_FEEDBACKTEXTURE:
        return ShaderParameterType::UAV;
    case D3D_SIT_RTACCELERATIONSTRUCTURE:
        throw std::runtime_error("Ray tracing is not supported in D3D11!");
    default:
        throw std::runtime_error("Unsupported ShaderInputType!");
    }
}

void D3D11ResourceBindingList::Reflect(const PrismObj<Blob>& shader, ShaderStage stage)
{
    if (!shader)
    {
        rangesSRVs.emplace_back(stage, ShaderParameterType::SRV, nullptr, 0);
        rangesUAVs.emplace_back(stage, ShaderParameterType::UAV, nullptr, 0);
        rangesCBVs.emplace_back(stage, ShaderParameterType::CBV, nullptr, 0);
        rangesSamplers.emplace_back(stage, ShaderParameterType::Sampler, nullptr, 0);
        rangesVariables.emplace_back();
        return;
    }

    ComPtr<ID3D11ShaderReflection> reflection;
    HRESULT hr = D3DReflect(shader->GetData(), shader->GetLength(), IID_PPV_ARGS(&reflection));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to reflect shader");
    }

    D3D11_SHADER_DESC shaderDesc;
    hr = reflection->GetDesc(&shaderDesc);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to get shader description");
    }

    std::vector<D3D11ShaderParameter> shaderParametersInStage;
    shaderParametersInStage.reserve(shaderDesc.BoundResources);

    for (uint32_t i = 0; i < shaderDesc.BoundResources; i++)
    {
        D3D11_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
        hr = reflection->GetResourceBindingDesc(i, &shaderInputBindDesc);
        if (FAILED(hr))
        {
            continue;
        }

        D3D11ShaderParameter parameter = {};
        parameter.index = shaderInputBindDesc.BindPoint;
        parameter.size = shaderInputBindDesc.BindCount;
        parameter.stage = stage;
        parameter.type = ConvertShaderInputType(shaderInputBindDesc.Type);

        size_t nameLen = std::strlen(shaderInputBindDesc.Name);
        parameter.name = static_cast<char*>(PrismAlloc(nameLen + 1));
        std::memcpy(parameter.name, shaderInputBindDesc.Name, nameLen + 1);
        parameter.hash = D3D11DescriptorRange::HashString(parameter.name);

        shaderParametersInStage.push_back(parameter);
    }

    std::vector<D3D11ShaderParameter> srvParams;
    std::vector<D3D11ShaderParameter> uavParams;
    std::vector<D3D11ShaderParameter> cbvParams;
    std::vector<D3D11ShaderParameter> samplerParams;

    for (const auto& param : shaderParametersInStage)
    {
        switch (param.type)
        {
        case ShaderParameterType::SRV:
            srvParams.push_back(param);
            break;
        case ShaderParameterType::UAV:
            uavParams.push_back(param);
            break;
        case ShaderParameterType::CBV:
            cbvParams.push_back(param);
            break;
        case ShaderParameterType::Sampler:
            samplerParams.push_back(param);
            break;
        }
    }

    rangesSRVs.emplace_back(stage, ShaderParameterType::SRV, srvParams.data(), static_cast<int>(srvParams.size()));
    rangesUAVs.emplace_back(stage, ShaderParameterType::UAV, uavParams.data(), static_cast<int>(uavParams.size()));
    rangesCBVs.emplace_back(stage, ShaderParameterType::CBV, cbvParams.data(), static_cast<int>(cbvParams.size()));
    rangesSamplers.emplace_back(stage, ShaderParameterType::Sampler, samplerParams.data(), static_cast<int>(samplerParams.size()));

    if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(PipelineStateFlags::ReflectVariables)) != 0)
    {
        // TODO: Implement variable reflection
        rangesVariables.emplace_back();
    }
}

void D3D11ResourceBindingList::Clear()
{
    rangesSRVs.clear();
    rangesUAVs.clear();
    rangesCBVs.clear();
    rangesSamplers.clear();
    rangesVariables.clear();
}

void D3D11ResourceBindingList::SetSRV(const char* name, ShaderResourceView* srv)
{
    void* p = srv ? static_cast<D3D11ShaderResourceView*>(srv)->GetView() : nullptr;
    for (auto& range : rangesSRVs)
    {
        range.TrySetByName(name, p);
    }
}

void D3D11ResourceBindingList::SetSRV(const char* name, void* srv)
{
    for (auto& range : rangesSRVs)
    {
        range.TrySetByName(name, srv);
    }
}

void D3D11ResourceBindingList::SetUAV(const char* name, UnorderedAccessView* uav, uint32_t initialCount)
{
    void* p = uav ? static_cast<D3D11UnorderedAccessView*>(uav)->GetView() : nullptr;
    for (auto& range : rangesUAVs)
    {
        range.TrySetByName(name, p, initialCount);
    }
}

void D3D11ResourceBindingList::SetUAV(const char* name, void* uav, uint32_t initialCount)
{
    for (auto& range : rangesUAVs)
    {
        range.TrySetByName(name, uav, initialCount);
    }
}

void D3D11ResourceBindingList::SetCBV(const char* name, Buffer* cbv)
{
    void* p = cbv ? static_cast<D3D11Buffer*>(cbv)->GetBuffer() : nullptr;
    for (auto& range : rangesCBVs)
    {
        range.TrySetByName(name, p);
    }
}

void D3D11ResourceBindingList::SetCBV(const char* name, void* cbv)
{
    for (auto& range : rangesCBVs)
    {
        range.TrySetByName(name, cbv);
    }
}

void D3D11ResourceBindingList::SetSampler(const char* name, SamplerState* sampler)
{
    void* p = sampler ? static_cast<D3D11SamplerState*>(sampler)->GetSamplerState() : nullptr;
    for (auto& range : rangesSamplers)
    {
        range.TrySetByName(name, p);
    }
}

void D3D11ResourceBindingList::SetSampler(const char* name, void* sampler)
{
    for (auto& range : rangesSamplers)
    {
        range.TrySetByName(name, sampler);
    }
}

void D3D11ResourceBindingList::SetSRV(const char* name, ShaderStage stage, ShaderResourceView* srv)
{
    void* p = srv ? static_cast<D3D11ShaderResourceView*>(srv)->GetView() : nullptr;
    rangesSRVs[static_cast<size_t>(stage)].TrySetByName(name, p);
}

void D3D11ResourceBindingList::SetUAV(const char* name, ShaderStage stage, UnorderedAccessView* uav, uint32_t initialCount)
{
    void* p = uav ? static_cast<D3D11UnorderedAccessView*>(uav)->GetView() : nullptr;
    rangesUAVs[static_cast<size_t>(stage)].TrySetByName(name, p, initialCount);
}

void D3D11ResourceBindingList::SetCBV(const char* name, ShaderStage stage, Buffer* cbv)
{
    void* p = cbv ? static_cast<D3D11Buffer*>(cbv)->GetBuffer() : nullptr;
    rangesCBVs[static_cast<size_t>(stage)].TrySetByName(name, p);
}

void D3D11ResourceBindingList::SetSampler(const char* name, ShaderStage stage, SamplerState* sampler)
{
    void* p = sampler ? static_cast<D3D11SamplerState*>(sampler)->GetSamplerState() : nullptr;
    rangesSamplers[static_cast<size_t>(stage)].TrySetByName(name, p);
}

void D3D11ResourceBindingList::UploadState(void* context)
{
    for (auto& var : rangesVariables)
    {
        var.Upload(context);
    }
}

#define DEFINE_BIND_FUNCTION(funcName, type) \
    static void funcName(const ComPtr<ID3D11DeviceContext3>& ctx, uint32_t startSlot, uint32_t count, void** resources) \
    { \
        ctx->funcName(startSlot, count, reinterpret_cast<type>(resources)); \
    }

DEFINE_BIND_FUNCTION(VSSetShaderResources, ID3D11ShaderResourceView* const*)
DEFINE_BIND_FUNCTION(HSSetShaderResources, ID3D11ShaderResourceView* const*)
DEFINE_BIND_FUNCTION(DSSetShaderResources, ID3D11ShaderResourceView* const*)
DEFINE_BIND_FUNCTION(GSSetShaderResources, ID3D11ShaderResourceView* const*)
DEFINE_BIND_FUNCTION(PSSetShaderResources, ID3D11ShaderResourceView* const*)

DEFINE_BIND_FUNCTION(VSSetConstantBuffers, ID3D11Buffer* const*)
DEFINE_BIND_FUNCTION(HSSetConstantBuffers, ID3D11Buffer* const*)
DEFINE_BIND_FUNCTION(DSSetConstantBuffers, ID3D11Buffer* const*)
DEFINE_BIND_FUNCTION(GSSetConstantBuffers, ID3D11Buffer* const*)
DEFINE_BIND_FUNCTION(PSSetConstantBuffers, ID3D11Buffer* const*)

DEFINE_BIND_FUNCTION(VSSetSamplers, ID3D11SamplerState* const*)
DEFINE_BIND_FUNCTION(HSSetSamplers, ID3D11SamplerState* const*)
DEFINE_BIND_FUNCTION(DSSetSamplers, ID3D11SamplerState* const*)
DEFINE_BIND_FUNCTION(GSSetSamplers, ID3D11SamplerState* const*)
DEFINE_BIND_FUNCTION(PSSetSamplers, ID3D11SamplerState* const*)

DEFINE_BIND_FUNCTION(CSSetShaderResources, ID3D11ShaderResourceView* const*)
DEFINE_BIND_FUNCTION(CSSetConstantBuffers, ID3D11Buffer* const*)
DEFINE_BIND_FUNCTION(CSSetSamplers, ID3D11SamplerState* const*)

void D3D11ResourceBindingList::BindGraphics(const ComPtr<ID3D11DeviceContext3>& context)
{
    rangesSRVs[static_cast<size_t>(ShaderStage::Vertex)].Bind(context, VSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Hull)].Bind(context, HSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Domain)].Bind(context, DSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Geometry)].Bind(context, GSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Pixel)].Bind(context, PSSetShaderResources);

    rangesCBVs[static_cast<size_t>(ShaderStage::Vertex)].Bind(context, VSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Hull)].Bind(context, HSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Domain)].Bind(context, DSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Geometry)].Bind(context, GSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Pixel)].Bind(context, PSSetConstantBuffers);

    rangesSamplers[static_cast<size_t>(ShaderStage::Vertex)].Bind(context, VSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Hull)].Bind(context, HSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Domain)].Bind(context, DSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Geometry)].Bind(context, GSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Pixel)].Bind(context, PSSetSamplers);
}

void D3D11ResourceBindingList::UnbindGraphics(const ComPtr<ID3D11DeviceContext3>& context)
{
    rangesSRVs[static_cast<size_t>(ShaderStage::Vertex)].Unbind(context, VSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Hull)].Unbind(context, HSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Domain)].Unbind(context, DSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Geometry)].Unbind(context, GSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Pixel)].Unbind(context, PSSetShaderResources);

    rangesCBVs[static_cast<size_t>(ShaderStage::Vertex)].Unbind(context, VSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Hull)].Unbind(context, HSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Domain)].Unbind(context, DSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Geometry)].Unbind(context, GSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Pixel)].Unbind(context, PSSetConstantBuffers);

    rangesSamplers[static_cast<size_t>(ShaderStage::Vertex)].Unbind(context, VSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Hull)].Unbind(context, HSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Domain)].Unbind(context, DSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Geometry)].Unbind(context, GSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Pixel)].Unbind(context, PSSetSamplers);
}

void D3D11ResourceBindingList::BindCompute(const ComPtr<ID3D11DeviceContext3>& context)
{
    rangesUAVs[0].BindUAV(context);
    rangesSRVs[0].Bind(context, CSSetShaderResources);
    rangesCBVs[0].Bind(context, CSSetConstantBuffers);
    rangesSamplers[0].Bind(context, CSSetSamplers);
}

void D3D11ResourceBindingList::UnbindCompute(const ComPtr<ID3D11DeviceContext3>& context)
{
    rangesUAVs[0].UnbindUAV(context);
    rangesSRVs[0].Unbind(context, CSSetShaderResources);
    rangesCBVs[0].Unbind(context, CSSetConstantBuffers);
    rangesSamplers[0].Unbind(context, CSSetSamplers);
}

HEXA_PRISM_NAMESPACE_END

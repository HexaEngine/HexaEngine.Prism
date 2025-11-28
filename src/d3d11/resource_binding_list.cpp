#include "resource_binding_list.hpp"
#include "d3d11.hpp"
#include <stdexcept>

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

D3D11ResourceBindingList::~D3D11ResourceBindingList() override
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
        for (size_t i = 0; i < rangesSRVs.size(); i++)
        {
            rangesSRVs[i].UpdateByName(name, oldState.Resource, state.Resource);
        }
        break;

    case ShaderParameterType::UAV:
        for (size_t i = 0; i < rangesUAVs.size(); i++)
        {
            rangesUAVs[i].UpdateByName(name, oldState.Resource, state.Resource, state.InitialCount);
        }
        break;

    case ShaderParameterType::CBV:
        for (size_t i = 0; i < rangesCBVs.size(); i++)
        {
            rangesCBVs[i].UpdateByName(name, oldState.Resource, state.Resource);
        }
        break;

    case ShaderParameterType::Sampler:
        for (size_t i = 0; i < rangesSamplers.size(); i++)
        {
            rangesSamplers[i].UpdateByName(name, oldState.Resource, state.Resource);
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

void D3D11ResourceBindingList::Reflect(void* shader, ShaderStage stage)
{
    // TODO: Implement shader reflection using ID3D11ShaderReflection
}

void D3D11ResourceBindingList::Clear()
{
    rangesSRVs.clear();
    rangesUAVs.clear();
    rangesCBVs.clear();
    rangesSamplers.clear();
    rangesVariables.clear();
}

void D3D11ResourceBindingList::SetSRV(const char* name, ShaderResourceView* srv) override
{
    void* p = srv ? static_cast<D3D11ShaderResourceView*>(srv)->GetView() : nullptr;
    for (size_t i = 0; i < rangesSRVs.size(); i++)
    {
        rangesSRVs[i].TrySetByName(name, p);
    }
}

void D3D11ResourceBindingList::SetSRV(const char* name, void* srv)
{
    for (size_t i = 0; i < rangesSRVs.size(); i++)
    {
        rangesSRVs[i].TrySetByName(name, srv);
    }
}

void D3D11ResourceBindingList::SetUAV(const char* name, UnorderedAccessView* uav, uint32_t initialCount) override
{
    void* p = uav ? static_cast<D3D11UnorderedAccessView*>(uav)->GetView() : nullptr;
    for (size_t i = 0; i < rangesUAVs.size(); i++)
    {
        rangesUAVs[i].TrySetByName(name, p, initialCount);
    }
}

void D3D11ResourceBindingList::SetUAV(const char* name, void* uav, uint32_t initialCount)
{
    for (size_t i = 0; i < rangesUAVs.size(); i++)
    {
        rangesUAVs[i].TrySetByName(name, uav, initialCount);
    }
}

void D3D11ResourceBindingList::SetCBV(const char* name, Buffer* cbv) override
{
    void* p = cbv ? static_cast<D3D11Buffer*>(cbv)->GetBuffer() : nullptr;
    for (size_t i = 0; i < rangesCBVs.size(); i++)
    {
        rangesCBVs[i].TrySetByName(name, p);
    }
}

void D3D11ResourceBindingList::SetCBV(const char* name, void* cbv)
{
    for (size_t i = 0; i < rangesCBVs.size(); i++)
    {
        rangesCBVs[i].TrySetByName(name, cbv);
    }
}

void D3D11ResourceBindingList::SetSampler(const char* name, SamplerState* sampler) override
{
    void* p = sampler ? static_cast<D3D11SamplerState*>(sampler)->GetSamplerState() : nullptr;
    for (size_t i = 0; i < rangesSamplers.size(); i++)
    {
        rangesSamplers[i].TrySetByName(name, p);
    }
}

void D3D11ResourceBindingList::SetSampler(const char* name, void* sampler)
{
    for (size_t i = 0; i < rangesSamplers.size(); i++)
    {
        rangesSamplers[i].TrySetByName(name, sampler);
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
    for (size_t i = 0; i < rangesVariables.size(); i++)
    {
        rangesVariables[i].Upload(context);
    }
}

#define DEFINE_BIND_FUNCTION(funcName, type) \
    static void funcName(const ComPtr<ID3D11DeviceContext3>& ctx, auto startSlot, auto count, auto resources) \
    { \
        ctx->funcName(startSlot, count, reinterpret_cast<type>(resources)); \
    }

DEFINE_BIND_FUNCTION(VSSetShaderResources, ID3D11ShaderResourceView**);
DEFINE_BIND_FUNCTION(HSSetShaderResources, ID3D11ShaderResourceView**);
DEFINE_BIND_FUNCTION(DSSetShaderResources, ID3D11ShaderResourceView**);
DEFINE_BIND_FUNCTION(GSSetShaderResources, ID3D11ShaderResourceView**);
DEFINE_BIND_FUNCTION(PSSetShaderResources, ID3D11ShaderResourceView**);

DEFINE_BIND_FUNCTION(VSSetConstantBuffers, ID3D11Buffer**);
DEFINE_BIND_FUNCTION(HSSetConstantBuffers, ID3D11Buffer**);
DEFINE_BIND_FUNCTION(DSSetConstantBuffers, ID3D11Buffer**);
DEFINE_BIND_FUNCTION(GSSetConstantBuffers, ID3D11Buffer**);
DEFINE_BIND_FUNCTION(PSSetConstantBuffers, ID3D11Buffer**);

DEFINE_BIND_FUNCTION(VSSetSamplers, ID3D11SamplerState**);
DEFINE_BIND_FUNCTION(HSSetSamplers, ID3D11SamplerState**);
DEFINE_BIND_FUNCTION(DSSetSamplers, ID3D11SamplerState**);
DEFINE_BIND_FUNCTION(GSSetSamplers, ID3D11SamplerState**);
DEFINE_BIND_FUNCTION(PSSetSamplers, ID3D11SamplerState**);

DEFINE_BIND_FUNCTION(CSSetShaderResources, ID3D11ShaderResourceView**);
DEFINE_BIND_FUNCTION(CSSetConstantBuffers, ID3D11Buffer**);
DEFINE_BIND_FUNCTION(CSSetSamplers, ID3D11SamplerState**);

void D3D11ResourceBindingList::BindGraphics(const ComPtr<ID3D11DeviceContext3>& context)
{
    // SRV
    rangesSRVs[static_cast<size_t>(ShaderStage::Vertex)].Bind(context, VSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Hull)].Bind(context, HSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Domain)].Bind(context, DSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Geometry)].Bind(context, GSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Pixel)].Bind(context, PSSetShaderResources);

    // CBV
    rangesCBVs[static_cast<size_t>(ShaderStage::Vertex)].Bind(context, VSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Hull)].Bind(context, HSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Domain)].Bind(context, DSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Geometry)].Bind(context, GSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Pixel)].Bind(context, PSSetConstantBuffers);

    // Sampler
    rangesSamplers[static_cast<size_t>(ShaderStage::Vertex)].Bind(context, VSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Hull)].Bind(context, HSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Domain)].Bind(context, DSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Geometry)].Bind(context, GSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Pixel)].Bind(context, PSSetSamplers);
}

void D3D11ResourceBindingList::UnbindGraphics(const ComPtr<ID3D11DeviceContext3>& context)
{
    // SRV
    rangesSRVs[static_cast<size_t>(ShaderStage::Vertex)].Unbind(context, VSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Hull)].Unbind(context, HSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Domain)].Unbind(context, DSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Geometry)].Unbind(context, GSSetShaderResources);
    rangesSRVs[static_cast<size_t>(ShaderStage::Pixel)].Unbind(context, PSSetShaderResources);

    // CBV
    rangesCBVs[static_cast<size_t>(ShaderStage::Vertex)].Unbind(context, VSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Hull)].Unbind(context, HSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Domain)].Unbind(context, DSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Geometry)].Unbind(context, GSSetConstantBuffers);
    rangesCBVs[static_cast<size_t>(ShaderStage::Pixel)].Unbind(context, PSSetConstantBuffers);

    // Sampler
    rangesSamplers[static_cast<size_t>(ShaderStage::Vertex)].Unbind(context, VSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Hull)].Unbind(context, HSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Domain)].Unbind(context, DSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Geometry)].Unbind(context, GSSetSamplers);
    rangesSamplers[static_cast<size_t>(ShaderStage::Pixel)].Unbind(context, PSSetSamplers);
}

void D3D11ResourceBindingList::BindCompute(const ComPtr<ID3D11DeviceContext3>& context)
{
    auto contextPtr = context.Get();
    
    // UAV
    rangesUAVs[0].BindUAV(context);

    // SRV
    rangesSRVs[0].Bind(context, CSSetShaderResources);

    // CBV
    rangesCBVs[0].Bind(context, CSSetConstantBuffers);

    // Sampler
    rangesSamplers[0].Bind(context, CSSetSamplers);
}

void D3D11ResourceBindingList::UnbindCompute(const ComPtr<ID3D11DeviceContext3>& context)
{
    auto contextPtr = context.Get();
    
    // UAV
    rangesUAVs[0].UnbindUAV(context);

    // SRV
    rangesSRVs[0].Unbind(context, CSSetShaderResources);

    // CBV
    rangesCBVs[0].Unbind(context, CSSetConstantBuffers);

    // Sampler
    rangesSamplers[0].Unbind(context, CSSetSamplers);
}

HEXA_PRISM_NAMESPACE_END

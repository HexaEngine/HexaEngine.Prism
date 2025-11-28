#pragma once
#include "descriptor_range.hpp"
#include "d3d11.hpp"
#include <vector>
#include <functional>
#include <string>

HEXA_PRISM_NAMESPACE_BEGIN

enum class PipelineStateFlags
{
    None = 0,
    ReflectVariables = 1 << 0,
};

struct D3D11ShaderParameterState
{
    ShaderParameterType Type;
    void* Resource;
    uint32_t InitialCount;
};

struct D3D11VariableListRange
{
    void Release() {}
    bool TrySetByName(const char* name, const void* value) { return false; }
    void Upload(void* context) {}
};

class D3D11GlobalResourceList
{
public:
    using StateChangedHandlerList = EventHandlerList<std::function<void(const char*, D3D11ShaderParameterState, D3D11ShaderParameterState)>>;
    static StateChangedHandlerList StateChanged;
    static void SetState(void* bindingList) {}
};

class D3D11GraphicsPipeline;
class D3D11ComputePipeline;

class D3D11ResourceBindingList : public ResourceBindingList
{
private:
    Pipeline* pipeline;
    PipelineStateFlags flags;
    std::vector<D3D11DescriptorRange> rangesSRVs;
    std::vector<D3D11DescriptorRange> rangesUAVs;
    std::vector<D3D11DescriptorRange> rangesCBVs;
    std::vector<D3D11DescriptorRange> rangesSamplers;
    std::vector<D3D11VariableListRange> rangesVariables;

    EventHandlerList<std::function<void(Pipeline*)>>::EventHandlerToken onCompileToken;
    D3D11GlobalResourceList::StateChangedHandlerList::EventHandlerToken globalStateChangedToken;

public:
    D3D11ResourceBindingList(D3D11GraphicsPipeline* pipeline, PipelineStateFlags flags);
    D3D11ResourceBindingList(D3D11ComputePipeline* pipeline, PipelineStateFlags flags);
    ~D3D11ResourceBindingList() override;

    Pipeline* GetPipeline() const override { return pipeline; }

    void Hook();

private:
    void GlobalStateChanged(const char* name, D3D11ShaderParameterState oldState, D3D11ShaderParameterState state);
    void OnPipelineCompile(Pipeline* pipeline);
    D3D11GraphicsDevice* GetDevice();
    void Reflect(const PrismObj<Blob>& shader, ShaderStage stage);
    void Clear();

public:
    void SetSRV(const char* name, ShaderResourceView* srv) override;
    void SetSRV(const char* name, void* srv);
    void SetUAV(const char* name, UnorderedAccessView* uav, uint32_t initialCount) override;
    void SetUAV(const char* name, void* uav, uint32_t initialCount = static_cast<uint32_t>(-1));
    void SetCBV(const char* name, Buffer* cbv) override;
    void SetCBV(const char* name, void* cbv);
    void SetSampler(const char* name, SamplerState* sampler) override;
    void SetSampler(const char* name, void* sampler);

    template<typename T>
    void SetVariable(const char* name, const T& value)
    {
        for (auto& var : rangesVariables)
        {
            var.TrySetByName(name, &value);
        }
    }

    void SetSRV(const char* name, ShaderStage stage, ShaderResourceView* srv) override;
    void SetUAV(const char* name, ShaderStage stage, UnorderedAccessView* uav, uint32_t initialCount = static_cast<uint32_t>(-1)) override;
    void SetCBV(const char* name, ShaderStage stage, Buffer* cbv) override;
    void SetSampler(const char* name, ShaderStage stage, SamplerState* sampler) override;

    template<typename T>
    void SetVariable(const char* name, ShaderStage stage, const T& value)
    {
        rangesVariables[static_cast<size_t>(stage)].TrySetByName(name, &value);
    }

    void UploadState(void* context);

    void BindGraphics(const ComPtr<ID3D11DeviceContext3>& context);
    void UnbindGraphics(const ComPtr<ID3D11DeviceContext3>& context);
    void BindCompute(const ComPtr<ID3D11DeviceContext3>& context);
    void UnbindCompute(const ComPtr<ID3D11DeviceContext3>& context);
};

HEXA_PRISM_NAMESPACE_END
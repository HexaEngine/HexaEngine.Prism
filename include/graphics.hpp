#pragma once
#include "common.hpp"
#include <cstdint>

HEXA_PRISM_NAMESPACE_BEGIN

class PrismObject
{
    std::atomic<size_t> counter;
public:
    void AddRef()
    {
        counter.fetch_add(1, std::memory_order_acq_rel);
    }

    void Release()
    {
        if (counter.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete this; // TODO: Change to custom allocator solution.
        }
    }

    virtual ~PrismObject() = default;
};

template <typename T>
class PrismObj
{
    T* ptr;

    public:
    constexpr PrismObj() : ptr(nullptr) {}
    
    explicit PrismObj(T* p, bool addRef = true) noexcept : ptr(p)
    {
        if (ptr && addRef) ptr->AddRef();
    }

    PrismObj(const PrismObj<T>& other) noexcept : ptr(other.ptr)
    {
        if (ptr) ptr->AddRef();
    }

    template <typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    PrismObj(const PrismObj<U>& other) noexcept : ptr(other.Get())
    {
        if (ptr) ptr->AddRef();
    }

    template <typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    PrismObj(PrismObj<U>& other) noexcept : ptr(other.Get())
    {
        if (ptr) ptr->AddRef();
    }

    PrismObj(PrismObj<T>&& other) noexcept : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    ~PrismObj() noexcept
    {
        if (ptr) ptr->Release();
        ptr = nullptr;
    }

    PrismObj<T>& operator=(const PrismObj<T>& other)
    {
        if (this != &other)
        {
            if (other.ptr) other.ptr->AddRef();
            if (ptr) ptr->Release();
            ptr = other.ptr;
        }
        return *this;
    }

    PrismObj<T>& operator=(PrismObj<T>&& other) noexcept
    {
        if (this != &other)
        {
            if (ptr) ptr->Release();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    PrismObj<T>& operator=(T* p) noexcept
    {
        if (ptr != p)
        {
            if (ptr) ptr->Release();
            ptr = p;
        }
        return *this;
    }

    template<typename U>
    constexpr operator U*() { return ptr; }

    constexpr T* operator->() { return ptr; }
    constexpr T& operator*() { return *ptr; }
    constexpr operator bool() const noexcept { return ptr != nullptr; }
    bool operator==(const PrismObj<T>& other) const noexcept { return ptr == other.ptr; }
    bool operator!=(const PrismObj<T>& other) const noexcept { return ptr != other.ptr; }
    bool operator==(T* p) const noexcept { return ptr == p; }
    bool operator!=(T* p) const noexcept { return ptr != p; }

    constexpr T* Get() { return ptr; }

    PrismObj<T> AddRef()
    {
        return PrismObj<T>(ptr, true);
    }

    T* Detach()
    {
        auto* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void Release()
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    void Reset(T* ptr)
    {
        Release();
        this->ptr = ptr;
    }

    void swap(PrismObj<T>& other) noexcept
    {
        std::swap(ptr, other.ptr);
    }
};

class Resource : public PrismObject
{

};

enum class Format
{
    Unknown,
    RGBA8_UNorm,
    RGBA16_UNorm,
    RGBA32_UNorm,
    RGB10A2_UNorm,
    RGBA8_Float,
    RGBA16_Float,
    RGBA32_Float,
    RGBA8_SNorm,
    RGBA16_SNorm,
    RGBA32_SNorm,
    R32_Float,
    RG32_Float,
    RGB32_Float,
    R16_Float,
    RG16_Float,
    RGB16_Float,
    R8G8_UNorm,
    
    BC1_UNorm,
    BC2_UNorm,
    BC3_UNorm,
    BC4_UNorm,
    BC5_UNorm,
    BC7_UNorm,
};

enum class CpuAccessFlags
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    All = Read | Write
};

enum class GpuAccessFlags
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    UA = 1 << 2,
    DepthStencil = 1 << 3,
    Immutable = 1 << 4,
};

struct SampleDesc
{
    uint32_t count;
    uint32_t quality;
};

enum class ResourceMiscFlags
{
    None = 0,
    TextureCube = 1 << 0,
};

struct Texture1DDesc
{
    GpuAccessFlags gpuAccessFlags;
    CpuAccessFlags cpuAccessFlags;
    Format format;
    uint32_t width;
    uint32_t arraySize;
    uint32_t mipLevels;
    ResourceMiscFlags miscFlags;
};

class Texture1D : public Resource
{
};

struct Texture2DDesc
{
    GpuAccessFlags gpuAccessFlags;
    CpuAccessFlags cpuAccessFlags;
    Format format;
    uint32_t width;
    uint32_t height;
    uint32_t arraySize;
    uint32_t mipLevels;
    SampleDesc sampleDesc;
    ResourceMiscFlags miscFlags;
};

class Texture2D : public Resource
{

};

struct Texture3DDesc
{
    GpuAccessFlags gpuAccessFlags;
    CpuAccessFlags cpuAccessFlags;
    Format format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevels;
    ResourceMiscFlags miscFlags;
};

class Texture3D : public Resource
{

};

enum class BufferType
{
    Default,
    ConstantBuffer,
};

struct BufferDesc
{
    BufferType type;
    uint32_t widthInBytes;
    uint32_t structureStride;
    CpuAccessFlags cpuAccessFlags;
    GpuAccessFlags gpuAccessFlags;
};

class Buffer : public Resource
{
    BufferDesc desc;

    Buffer(const BufferDesc& desc) : desc(desc) {}

    const BufferDesc& GetDesc() const { return desc; }
};

class PipelineState : public PrismObject
{

};

struct GraphicsPipelineDesc
{
};

class GraphicsPipeline : public PrismObject
{
};

struct GraphicsPipelineStateDesc 
{

};

class GraphicsPipelineState : public PipelineState
{
};

struct ComputePipelineDesc
{

};

class ComputePipeline : public PrismObject
{

};

struct ComputePipelineStateDesc
{

};

class ComputePipelineState : public PipelineState
{

};

class ResourceView : public PrismObject
{
};

class RenderTargetView : public ResourceView
{
};

class ShaderResourceView : public ResourceView
{
};

class DepthStencilView : public ResourceView
{
};

class UnorderedAccessView : public ResourceView
{
};

struct Viewport
{
    float X, Y;
    float Width, Height;
    float MinDepth, MaxDepth;
};

struct Color
{
    float r,g,b,a;
};

enum class DepthStencilViewClearFlags
{
    None = 0,
    Depth = 1 << 0,
    Stencil = 1 << 1,
    All = Depth | Stencil,
};

enum class CommandListType
{
    Immediate,
    Deferred,
};

class CommandList : public PrismObject
{
    virtual CommandListType GetType() const noexcept = 0;
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void SetGraphicsPipelineState(GraphicsPipelineState* state) = 0;
    virtual void SetComputePipelineState(ComputePipelineState* state) = 0;
    virtual void SetVertexBuffer(uint32_t slot, Buffer* buffer, uint32_t stride, uint32_t offset) = 0;
    virtual void SetIndexBuffer(Buffer* buffer, Format format, uint32_t offset) = 0;
    virtual void SetRenderTarget(RenderTargetView* rtv, DepthStencilView* dsv) = 0;
    virtual void SetRenderTargetsAndUnorderedAccessViews(uint32_t count, RenderTargetView** views, DepthStencilView* depthStencilView, uint32_t uavSlot, uint32_t uavCount, UnorderedAccessView** uavs, uint32_t* pUavInitialCount) = 0;
    virtual void SetViewport(const Viewport& viewport) = 0;
    virtual void SetViewports(uint32_t viewportCount, const Viewport* viewports) = 0;
    virtual void SetScissors(int32_t x, int32_t y, int32_t z, int32_t w) = 0;
    virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset) = 0;
    virtual void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, int32_t vertexOffset, uint32_t instanceOffset) = 0;
    virtual void DrawIndexedInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs) = 0;
    virtual void DrawInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs) = 0;
    virtual void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;
    virtual void DispatchIndirect(Buffer* dispatchArgs, uint32_t offset) = 0;
    virtual void ExecuteCommandList(CommandList* commandList) = 0;
    virtual void ClearRenderTargetView(RenderTargetView* rtv, const Color& color) = 0;
    virtual void ClearDepthStencilView(DepthStencilView* dsv, DepthStencilViewClearFlags flags, float depth, char stencil) = 0;
};

class GraphicsDevice
{
    virtual PrismObj<Buffer> CreateBuffer(const BufferDesc& desc) = 0;
    virtual PrismObj<Texture1D> CreateTexture1D(const Texture1DDesc& desc) = 0;
    virtual PrismObj<Texture2D> CreateTexture2D(const Texture2DDesc& desc) = 0;
    virtual PrismObj<Texture3D> CreateTexture3D(const Texture3DDesc& desc) = 0;
    virtual PrismObj<CommandList> CreateCommandList() = 0;
    virtual PrismObj<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
    virtual PrismObj<GraphicsPipelineState> CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc) = 0;
};

HEXA_PRISM_NAMESPACE_END
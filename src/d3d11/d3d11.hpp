#pragma once
#include "common.hpp"
#include "graphics.hpp"
#include <dxgi.h>
#include <d3d11_4.h>
#include <wrl/client.h>
#include "descriptor_range.hpp"

HEXA_PRISM_NAMESPACE_BEGIN

using Microsoft::WRL::ComPtr;

class D3D11Buffer : public Buffer
{
	ComPtr<ID3D11Buffer> buffer;

public:
	D3D11Buffer(const BufferDesc& desc, ComPtr<ID3D11Buffer>&& buffer);
	~D3D11Buffer() override = default;

	ID3D11Buffer* GetBuffer() const { return buffer.Get(); }
};

class D3D11Texture1D : public Texture1D
{
	ComPtr<ID3D11Texture1D> texture;

public:
	D3D11Texture1D(const Texture1DDesc& desc, ComPtr<ID3D11Texture1D>&& texture);
	~D3D11Texture1D() override = default;

	ID3D11Texture1D* GetTexture() const { return texture.Get(); }
};

class D3D11Texture2D : public Texture2D
{
	ComPtr<ID3D11Texture2D> texture;

public:
	D3D11Texture2D(const Texture2DDesc& desc, ComPtr<ID3D11Texture2D>&& texture);
	~D3D11Texture2D() override = default;

	ID3D11Texture2D* GetTexture() const { return texture.Get(); }
};

class D3D11Texture3D : public Texture3D
{
	ComPtr<ID3D11Texture3D> texture;

public:
	D3D11Texture3D(const Texture3DDesc& desc, ComPtr<ID3D11Texture3D>&& texture);
	~D3D11Texture3D() override = default;

	ID3D11Texture3D* GetTexture() const { return texture.Get(); }
};

class D3D11RenderTargetView : public RenderTargetView
{
	ComPtr<ID3D11RenderTargetView> view;

public:
	D3D11RenderTargetView(const RenderTargetViewDesc& desc, ComPtr<ID3D11RenderTargetView>&& view);
	~D3D11RenderTargetView() override = default;

	ID3D11RenderTargetView* GetView() const { return view.Get(); }
};

class D3D11ShaderResourceView : public ShaderResourceView
{
	ComPtr<ID3D11ShaderResourceView> view;

public:
	D3D11ShaderResourceView(const ShaderResourceViewDesc& desc, ComPtr<ID3D11ShaderResourceView>&& view);
	~D3D11ShaderResourceView() override = default;

	ID3D11ShaderResourceView* GetView() const { return view.Get(); }
};

class D3D11DepthStencilView : public DepthStencilView
{
	ComPtr<ID3D11DepthStencilView> view;

public:
	D3D11DepthStencilView(const DepthStencilViewDesc& desc, ComPtr<ID3D11DepthStencilView>&& view);
	~D3D11DepthStencilView() override = default;

	ID3D11DepthStencilView* GetView() const { return view.Get(); }
};

class D3D11UnorderedAccessView : public UnorderedAccessView
{
	ComPtr<ID3D11UnorderedAccessView> view;

public:
	D3D11UnorderedAccessView(const UnorderedAccessViewDesc& desc, ComPtr<ID3D11UnorderedAccessView>&& view);
	~D3D11UnorderedAccessView() override = default;

	ID3D11UnorderedAccessView* GetView() const { return view.Get(); }
};

class D3D11SamplerState : public SamplerState
{
	ComPtr<ID3D11SamplerState> samplerState;

public:
	D3D11SamplerState(const SamplerDesc& desc, ComPtr<ID3D11SamplerState>&& samplerState);
	~D3D11SamplerState() override = default;

	ID3D11SamplerState* GetSamplerState() const { return samplerState.Get(); }
};

class D3D11GraphicsPipeline : public GraphicsPipeline
{
	friend class D3D11ResourceBindingList;
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







};

class D3D11ComputePipeline : public ComputePipeline
{
	friend class D3D11ResourceBindingList;
	ComPtr<ID3D11ComputeShader> cs;

	PrismObj<Blob> computeShaderBlob;
};

class D3D11CommandList : public CommandList
{
	ComPtr<ID3D11DeviceContext4> context;
	ComPtr<ID3D11CommandList> commandList;
	CommandListType type;

public:
	D3D11CommandList(ComPtr<ID3D11DeviceContext4>&& context, CommandListType type);
	~D3D11CommandList() override = default;

	CommandListType GetType() const noexcept override;
	void Begin() override;
	void End() override;
	void SetGraphicsPipelineState(GraphicsPipelineState* state) override;
	void SetComputePipelineState(ComputePipelineState* state) override;
	void SetVertexBuffer(uint32_t slot, Buffer* buffer, uint32_t stride, uint32_t offset) override;
	void SetIndexBuffer(Buffer* buffer, Format format, uint32_t offset) override;
	void SetRenderTarget(RenderTargetView* rtv, DepthStencilView* dsv) override;
	void SetRenderTargetsAndUnorderedAccessViews(uint32_t count, RenderTargetView** views, DepthStencilView* depthStencilView, uint32_t uavSlot, uint32_t uavCount, UnorderedAccessView** uavs, uint32_t* pUavInitialCount) override;
	void SetViewport(const Viewport& viewport) override;
	void SetViewports(uint32_t viewportCount, const Viewport* viewports) override;
	void SetScissors(int32_t x, int32_t y, int32_t z, int32_t w) override;
	void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset) override;
	void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, int32_t vertexOffset, uint32_t instanceOffset) override;
	void DrawIndexedInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs) override;
	void DrawInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs) override;
	void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;
	void DispatchIndirect(Buffer* dispatchArgs, uint32_t offset) override;
	void ExecuteCommandList(CommandList* commandList) override;
	void ClearRenderTargetView(RenderTargetView* rtv, const Color& color) override;
	void ClearDepthStencilView(DepthStencilView* dsv, DepthStencilViewClearFlags flags, float depth, char stencil) override;

	ID3D11DeviceContext4* GetContext() const { return context.Get(); }
};

class D3D11GraphicsDevice : public GraphicsDevice
{
	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGIAdapter3> adapter;
	ComPtr<ID3D11Device4> device;
	PrismObj<D3D11CommandList> immediateContext;

public:
	D3D11GraphicsDevice() = default;
	~D3D11GraphicsDevice() override = default;

	bool Initialize();

	CommandList* GetImmediateCommandList() override;
	PrismObj<Buffer> CreateBuffer(const BufferDesc& desc) override;
	PrismObj<Texture1D> CreateTexture1D(const Texture1DDesc& desc) override;
	PrismObj<Texture2D> CreateTexture2D(const Texture2DDesc& desc) override;
	PrismObj<Texture3D> CreateTexture3D(const Texture3DDesc& desc) override;
	PrismObj<RenderTargetView> CreateRenderTargetView(Resource* resource, const RenderTargetViewDesc& desc) override;
	PrismObj<ShaderResourceView> CreateShaderResourceView(Resource* resource, const ShaderResourceViewDesc& desc) override;
	PrismObj<DepthStencilView> CreateDepthStencilView(Resource* resource, const DepthStencilViewDesc& desc) override;
	PrismObj<UnorderedAccessView> CreateUnorderedAccessView(Resource* resource, const UnorderedAccessViewDesc& desc) override;
	PrismObj<SamplerState> CreateSamplerState(const SamplerDesc& desc) override;
	PrismObj<CommandList> CreateCommandList() override;
	PrismObj<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
	PrismObj<GraphicsPipelineState> CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc) override;
	PrismObj<ComputePipeline> CreateComputePipeline(const ComputePipelineDesc& desc) override;
	PrismObj<ComputePipelineState> CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc) override;

	ID3D11Device4* GetDevice() const { return device.Get(); }
	IDXGIFactory4* GetFactory() const { return factory.Get(); }
	IDXGIAdapter3* GetAdapter() const { return adapter.Get(); }
};

HEXA_PRISM_NAMESPACE_END
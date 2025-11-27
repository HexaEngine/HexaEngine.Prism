#include "d3d11.hpp"
#include <stdexcept>

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
	DXGI_FORMAT ConvertFormat(const Format format)
	{
		switch (format)
		{
		case Format::Unknown:
			return DXGI_FORMAT_UNKNOWN;
		case Format::RGBA8_UNorm:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA16_UNorm:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case Format::RGBA32_UNorm:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case Format::RGB10A2_UNorm:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case Format::RGBA16_Float:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case Format::RGBA32_Float:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Format::RGBA8_SNorm:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case Format::RGBA16_SNorm:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case Format::RGBA32_SNorm:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case Format::R32_Float:
			return DXGI_FORMAT_R32_FLOAT;
		case Format::RG32_Float:
			return DXGI_FORMAT_R32G32_FLOAT;
		case Format::RGB32_Float:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case Format::R16_Float:
			return DXGI_FORMAT_R16_FLOAT;
		case Format::RG16_Float:
			return DXGI_FORMAT_R16G16_FLOAT;
		case Format::R8G8_UNorm:
			return DXGI_FORMAT_R8G8_UNORM;
		case Format::BC1_UNorm:
			return DXGI_FORMAT_BC1_UNORM;
		case Format::BC2_UNorm:
			return DXGI_FORMAT_BC2_UNORM;
		case Format::BC3_UNorm:
			return DXGI_FORMAT_BC3_UNORM;
		case Format::BC4_UNorm:
			return DXGI_FORMAT_BC4_UNORM;
		case Format::BC5_UNorm:
			return DXGI_FORMAT_BC5_UNORM;
		case Format::BC7_UNorm:
			return DXGI_FORMAT_BC7_UNORM;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D11_USAGE ConvertUsage(CpuAccessFlags cpuAccess, GpuAccessFlags gpuAccess)
	{
		const bool cpuRead = (static_cast<uint32_t>(cpuAccess) & static_cast<uint32_t>(CpuAccessFlags::Read)) != 0;
		const bool cpuWrite = (static_cast<uint32_t>(cpuAccess) & static_cast<uint32_t>(CpuAccessFlags::Write)) != 0;
		const bool immutable = (static_cast<uint32_t>(gpuAccess) & static_cast<uint32_t>(GpuAccessFlags::Immutable)) != 0;

		if (immutable)
		{
			return D3D11_USAGE_IMMUTABLE;
		}
		else if (cpuRead)
		{
			return D3D11_USAGE_STAGING;
		}
		else if (cpuWrite)
		{
			return D3D11_USAGE_DYNAMIC;
		}
		else
		{
			return D3D11_USAGE_DEFAULT;
		}
	}

	UINT ConvertCpuAccessFlags(CpuAccessFlags flags)
	{
		UINT result = 0;
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(CpuAccessFlags::Read)) != 0)
			result |= D3D11_CPU_ACCESS_READ;
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(CpuAccessFlags::Write)) != 0)
			result |= D3D11_CPU_ACCESS_WRITE;
		return result;
	}

	UINT ConvertBindFlags(GpuAccessFlags flags)
	{
		UINT result = 0;
		
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(GpuAccessFlags::Read)) != 0)
			result |= D3D11_BIND_SHADER_RESOURCE;
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(GpuAccessFlags::Write)) != 0)
			result |= D3D11_BIND_RENDER_TARGET;
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(GpuAccessFlags::UA)) != 0)
			result |= D3D11_BIND_UNORDERED_ACCESS;
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(GpuAccessFlags::DepthStencil)) != 0)
			result |= D3D11_BIND_DEPTH_STENCIL;
		
		return result;
	}

	UINT ConvertResourceMiscFlags(ResourceMiscFlags flags)
	{
		UINT result = 0;
		if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(ResourceMiscFlags::TextureCube)) != 0)
			result |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		return result;
	}
}

// D3D11Buffer Implementation

D3D11Buffer::D3D11Buffer(const BufferDesc& desc, ComPtr<ID3D11Buffer>&& buffer)
	: Buffer(desc), buffer(std::move(buffer))
{
}

// D3D11Texture1D Implementation

D3D11Texture1D::D3D11Texture1D(const Texture1DDesc& desc, ComPtr<ID3D11Texture1D>&& texture)
	: Texture1D(desc), texture(std::move(texture))
{
}

// D3D11Texture2D Implementation

D3D11Texture2D::D3D11Texture2D(const Texture2DDesc& desc, ComPtr<ID3D11Texture2D>&& texture)
	: Texture2D(desc), texture(std::move(texture))
{
}

// D3D11Texture3D Implementation

D3D11Texture3D::D3D11Texture3D(const Texture3DDesc& desc, ComPtr<ID3D11Texture3D>&& texture)
	: Texture3D(desc), texture(std::move(texture))
{
}

// D3D11RenderTargetView Implementation

D3D11RenderTargetView::D3D11RenderTargetView(const RenderTargetViewDesc& desc, ComPtr<ID3D11RenderTargetView>&& view)
	: RenderTargetView(desc), view(std::move(view))
{
}

// D3D11ShaderResourceView Implementation

D3D11ShaderResourceView::D3D11ShaderResourceView(const ShaderResourceViewDesc& desc, ComPtr<ID3D11ShaderResourceView>&& view)
	: ShaderResourceView(desc), view(std::move(view))
{
}

// D3D11DepthStencilView Implementation

D3D11DepthStencilView::D3D11DepthStencilView(const DepthStencilViewDesc& desc, ComPtr<ID3D11DepthStencilView>&& view)
	: DepthStencilView(desc), view(std::move(view))
{
}

// D3D11UnorderedAccessView Implementation

D3D11UnorderedAccessView::D3D11UnorderedAccessView(const UnorderedAccessViewDesc& desc, ComPtr<ID3D11UnorderedAccessView>&& view)
	: UnorderedAccessView(desc), view(std::move(view))
{
}

// D3D11SamplerState Implementation

D3D11SamplerState::D3D11SamplerState(const SamplerDesc& desc, ComPtr<ID3D11SamplerState>&& samplerState)
	: SamplerState(desc), samplerState(std::move(samplerState))
{
}

// D3D11CommandList Implementation

D3D11CommandList::D3D11CommandList(ComPtr<ID3D11DeviceContext4>&& context, const CommandListType type)
	: context(std::move(context)), type(type)
{
}

CommandListType D3D11CommandList::GetType() const noexcept
{
	return type;
}

void D3D11CommandList::Begin()
{
	commandList.Reset();
}

void D3D11CommandList::End()
{
	const auto hr = context->FinishCommandList(FALSE, &commandList);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to finish command list.");
	}
}

void D3D11CommandList::SetGraphicsPipelineState(GraphicsPipelineState* state)
{
	// TODO: Implement pipeline state setting
	// This would set all the pipeline state objects (shaders, blend state, rasterizer state, etc.)
}

void D3D11CommandList::SetComputePipelineState(ComputePipelineState* state)
{
	// TODO: Implement compute pipeline state setting
}

void D3D11CommandList::SetVertexBuffer(const uint32_t slot, Buffer* buffer, const uint32_t stride, const uint32_t offset)
{
	if (!buffer)
	{
		ID3D11Buffer* nullBuffer = nullptr;
		context->IASetVertexBuffers(slot, 1, &nullBuffer, &stride, &offset);
		return;
	}

	const auto d3dBuffer = static_cast<D3D11Buffer*>(buffer);
	ID3D11Buffer* d3dBufferPtr = d3dBuffer->GetBuffer();
	context->IASetVertexBuffers(slot, 1, &d3dBufferPtr, &stride, &offset);
}

void D3D11CommandList::SetIndexBuffer(Buffer* buffer, const Format format, const uint32_t offset)
{
	const DXGI_FORMAT dxgiFormat = ConvertFormat(format);
	
	if (!buffer)
	{
		context->IASetIndexBuffer(nullptr, dxgiFormat, offset);
		return;
	}

	const auto d3dBuffer = static_cast<D3D11Buffer*>(buffer);
	context->IASetIndexBuffer(d3dBuffer->GetBuffer(), dxgiFormat, offset);
}

void D3D11CommandList::SetRenderTarget(RenderTargetView* rtv, DepthStencilView* dsv)
{
	ID3D11RenderTargetView* d3dRtv = nullptr;
	ID3D11DepthStencilView* d3dDsv = nullptr;

	if (rtv)
	{
		const auto d3d11Rtv = static_cast<D3D11RenderTargetView*>(rtv);
		d3dRtv = d3d11Rtv->GetView();
	}

	if (dsv)
	{
		const auto d3d11Dsv = static_cast<D3D11DepthStencilView*>(dsv);
		d3dDsv = d3d11Dsv->GetView();
	}

	context->OMSetRenderTargets(1, &d3dRtv, d3dDsv);
}

void D3D11CommandList::SetRenderTargetsAndUnorderedAccessViews(
	const uint32_t count, 
	RenderTargetView** views, 
	DepthStencilView* depthStencilView, 
	const uint32_t uavSlot, 
	const uint32_t uavCount, 
	UnorderedAccessView** uavs, 
	uint32_t* pUavInitialCount)
{
	constexpr uint32_t maxRtvs = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	ID3D11RenderTargetView* d3dRtvs[maxRtvs] = {};
	
	for (uint32_t i = 0; i < count && i < maxRtvs; i++)
	{
		if (views[i])
		{
			const auto d3d11Rtv = static_cast<D3D11RenderTargetView*>(views[i]);
			d3dRtvs[i] = d3d11Rtv->GetView();
		}
	}

	ID3D11DepthStencilView* d3dDsv = nullptr;
	if (depthStencilView)
	{
		const auto d3d11Dsv = static_cast<D3D11DepthStencilView*>(depthStencilView);
		d3dDsv = d3d11Dsv->GetView();
	}

	constexpr uint32_t maxUavs = D3D11_1_UAV_SLOT_COUNT;
	ID3D11UnorderedAccessView* d3dUavs[maxUavs] = {};

	for (uint32_t i = 0; i < uavCount && i < maxUavs; i++)
	{
		if (uavs[i])
		{
			const auto d3d11Uav = static_cast<D3D11UnorderedAccessView*>(uavs[i]);
			d3dUavs[i] = d3d11Uav->GetView();
		}
	}

	context->OMSetRenderTargetsAndUnorderedAccessViews(count, d3dRtvs, d3dDsv, uavSlot, uavCount, d3dUavs, pUavInitialCount);
}

void D3D11CommandList::SetViewport(const Viewport& viewport)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = viewport.X;
	vp.TopLeftY = viewport.Y;
	vp.Width = viewport.Width;
	vp.Height = viewport.Height;
	vp.MinDepth = viewport.MinDepth;
	vp.MaxDepth = viewport.MaxDepth;
	context->RSSetViewports(1, &vp);
}

void D3D11CommandList::SetViewports(const uint32_t viewportCount, const Viewport* viewports)
{
	D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

	for (uint32_t i = 0; i < viewportCount; i++)
	{
		vps[i].TopLeftX = viewports[i].X;
		vps[i].TopLeftY = viewports[i].Y;
		vps[i].Width = viewports[i].Width;
		vps[i].Height = viewports[i].Height;
		vps[i].MinDepth = viewports[i].MinDepth;
		vps[i].MaxDepth = viewports[i].MaxDepth;
	}
	context->RSSetViewports(viewportCount, vps);
}

void D3D11CommandList::SetScissors(const int32_t x, const int32_t y, const int32_t z, const int32_t w)
{
	D3D11_RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = z;
	rect.bottom = w;
	context->RSSetScissorRects(1, &rect);
}

void D3D11CommandList::DrawInstanced(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t vertexOffset, const uint32_t instanceOffset)
{
	context->DrawInstanced(vertexCount, instanceCount, vertexOffset, instanceOffset);
}

void D3D11CommandList::DrawIndexedInstanced(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t indexOffset, const int32_t vertexOffset, const uint32_t instanceOffset)
{
	context->DrawIndexedInstanced(indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}

void D3D11CommandList::DrawIndexedInstancedIndirect(Buffer* bufferForArgs, const uint32_t alignedByteOffsetForArgs)
{
	const auto d3dBuffer = static_cast<D3D11Buffer*>(bufferForArgs);
	context->DrawIndexedInstancedIndirect(d3dBuffer->GetBuffer(), alignedByteOffsetForArgs);
}

void D3D11CommandList::DrawInstancedIndirect(Buffer* bufferForArgs, const uint32_t alignedByteOffsetForArgs)
{
	const auto d3dBuffer = static_cast<D3D11Buffer*>(bufferForArgs);
	context->DrawInstancedIndirect(d3dBuffer->GetBuffer(), alignedByteOffsetForArgs);
}

void D3D11CommandList::Dispatch(const uint32_t threadGroupCountX, const uint32_t threadGroupCountY, const uint32_t threadGroupCountZ)
{
	context->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void D3D11CommandList::DispatchIndirect(Buffer* dispatchArgs, const uint32_t offset)
{
	const auto d3dBuffer = static_cast<D3D11Buffer*>(dispatchArgs);
	context->DispatchIndirect(d3dBuffer->GetBuffer(), offset);
}

void D3D11CommandList::ExecuteCommandList(CommandList* commandList)
{
	const auto cmdList = static_cast<D3D11CommandList*>(commandList);
	context->ExecuteCommandList(cmdList->commandList.Get(), FALSE);
}

void D3D11CommandList::ClearRenderTargetView(RenderTargetView* rtv, const Color& color)
{
	const float clearColor[4] = { color.r, color.g, color.b, color.a };
	const auto d3d11Rtv = static_cast<D3D11RenderTargetView*>(rtv);
	context->ClearRenderTargetView(d3d11Rtv->GetView(), clearColor);
}

void D3D11CommandList::ClearDepthStencilView(DepthStencilView* dsv, const DepthStencilViewClearFlags flags, const float depth, const char stencil)
{
	UINT clearFlags = 0;
	if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(DepthStencilViewClearFlags::Depth)) != 0)
		clearFlags |= D3D11_CLEAR_DEPTH;
	if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(DepthStencilViewClearFlags::Stencil)) != 0)
		clearFlags |= D3D11_CLEAR_STENCIL;
	
	const auto d3d11Dsv = static_cast<D3D11DepthStencilView*>(dsv);
	context->ClearDepthStencilView(d3d11Dsv->GetView(), clearFlags, depth, static_cast<UINT8>(stencil));
}

// D3D11GraphicsDevice Implementation

bool D3D11GraphicsDevice::Initialize()
{
	HRESULT hr;

	// Create DXGI Factory
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IDXGIAdapter1> tempAdapter;
	for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &tempAdapter)); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		tempAdapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			tempAdapter.Reset();
			continue;
		}

		hr = tempAdapter.As(&adapter);
		if (SUCCEEDED(hr))
		{
			break;
		}

		tempAdapter.Reset();
	}

	if (!adapter)
	{
		return false;
	}

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	ComPtr<ID3D11Device> tempDevice;
	ComPtr<ID3D11DeviceContext> tempContext;

	hr = D3D11CreateDevice(
		adapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		createDeviceFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		&tempDevice,
		nullptr,
		&tempContext
	);

	if (FAILED(hr))
	{
		return false;
	}

	hr = tempDevice.As(&device);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<ID3D11DeviceContext4> ctx;
	hr = tempContext.As(&ctx);
	if (FAILED(hr))
	{
		return false;
	}

	immediateContext = MakePrismObj<D3D11CommandList>(ctx, CommandListType::Immediate);

	return true;
}

CommandList* D3D11GraphicsDevice::GetImmediateCommandList()
{
	return immediateContext.Get();
}

PrismObj<Buffer> D3D11GraphicsDevice::CreateBuffer(const BufferDesc& desc)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = desc.widthInBytes;
	bufferDesc.Usage = ConvertUsage(desc.cpuAccessFlags, desc.gpuAccessFlags);
	bufferDesc.CPUAccessFlags = ConvertCpuAccessFlags(desc.cpuAccessFlags);
	bufferDesc.StructureByteStride = desc.structureStride;

	switch (desc.type)
	{
	case BufferType::ConstantBuffer:
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		break;
	
	case BufferType::VertexBuffer:
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.BindFlags |= ConvertBindFlags(desc.gpuAccessFlags);
		break;
	
	case BufferType::IndexBuffer:
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.BindFlags |= ConvertBindFlags(desc.gpuAccessFlags);
		break;
	
	default:
		bufferDesc.BindFlags = ConvertBindFlags(desc.gpuAccessFlags);
		break;
	}

	ComPtr<ID3D11Buffer> d3dBuffer;
	const HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &d3dBuffer);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11Buffer>(desc, d3dBuffer);
}

PrismObj<Texture1D> D3D11GraphicsDevice::CreateTexture1D(const Texture1DDesc& desc)
{
	D3D11_TEXTURE1D_DESC texDesc = {};
	texDesc.Width = desc.width;
	texDesc.MipLevels = desc.mipLevels;
	texDesc.ArraySize = desc.arraySize;
	texDesc.Format = ConvertFormat(desc.format);
	texDesc.Usage = ConvertUsage(desc.cpuAccessFlags, desc.gpuAccessFlags);
	texDesc.BindFlags = ConvertBindFlags(desc.gpuAccessFlags);
	texDesc.CPUAccessFlags = ConvertCpuAccessFlags(desc.cpuAccessFlags);
	texDesc.MiscFlags = ConvertResourceMiscFlags(desc.miscFlags);

	ComPtr<ID3D11Texture1D> d3dTexture;
	const HRESULT hr = device->CreateTexture1D(&texDesc, nullptr, &d3dTexture);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11Texture1D>(desc, d3dTexture);
}

PrismObj<Texture2D> D3D11GraphicsDevice::CreateTexture2D(const Texture2DDesc& desc)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = desc.width;
	texDesc.Height = desc.height;
	texDesc.MipLevels = desc.mipLevels;
	texDesc.ArraySize = desc.arraySize;
	texDesc.Format = ConvertFormat(desc.format);
	texDesc.SampleDesc.Count = desc.sampleDesc.count;
	texDesc.SampleDesc.Quality = desc.sampleDesc.quality;
	texDesc.Usage = ConvertUsage(desc.cpuAccessFlags, desc.gpuAccessFlags);
	texDesc.BindFlags = ConvertBindFlags(desc.gpuAccessFlags);
	texDesc.CPUAccessFlags = ConvertCpuAccessFlags(desc.cpuAccessFlags);
	texDesc.MiscFlags = ConvertResourceMiscFlags(desc.miscFlags);

	ComPtr<ID3D11Texture2D> d3dTexture;
	const HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &d3dTexture);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11Texture2D>(desc, d3dTexture);
}

PrismObj<Texture3D> D3D11GraphicsDevice::CreateTexture3D(const Texture3DDesc& desc)
{
	D3D11_TEXTURE3D_DESC texDesc = {};
	texDesc.Width = desc.width;
	texDesc.Height = desc.height;
	texDesc.Depth = desc.depth;
	texDesc.MipLevels = desc.mipLevels;
	texDesc.Format = ConvertFormat(desc.format);
	texDesc.Usage = ConvertUsage(desc.cpuAccessFlags, desc.gpuAccessFlags);
	texDesc.BindFlags = ConvertBindFlags(desc.gpuAccessFlags);
	texDesc.CPUAccessFlags = ConvertCpuAccessFlags(desc.cpuAccessFlags);
	texDesc.MiscFlags = ConvertResourceMiscFlags(desc.miscFlags);

	ComPtr<ID3D11Texture3D> d3dTexture;
	const HRESULT hr = device->CreateTexture3D(&texDesc, nullptr, &d3dTexture);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11Texture3D>(desc, d3dTexture);
}

PrismObj<CommandList> D3D11GraphicsDevice::CreateCommandList()
{
	// Create a deferred context
	ComPtr<ID3D11DeviceContext> deferredContext;
	HRESULT hr = device->CreateDeferredContext(0, &deferredContext);
	if (FAILED(hr))
	{
		return {};
	}

	ComPtr<ID3D11DeviceContext4> deferredContext4;
	hr = deferredContext.As(&deferredContext4);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11CommandList>(deferredContext4, CommandListType::Deferred);
}

PrismObj<RenderTargetView> D3D11GraphicsDevice::CreateRenderTargetView(Resource* resource, const RenderTargetViewDesc& desc)
{
	if (!resource)
		return {};

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = ConvertFormat(desc.format);

	switch (desc.dimension)
	{
	case RenderTargetViewDimension::Buffer:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
		rtvDesc.Buffer.FirstElement = desc.buffer.firstElement;
		rtvDesc.Buffer.NumElements = desc.buffer.numElements;
		break;

	case RenderTargetViewDimension::Texture1D:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
		rtvDesc.Texture1D.MipSlice = desc.texture1D.mipSlice;
		break;

	case RenderTargetViewDimension::Texture1DArray:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		rtvDesc.Texture1DArray.MipSlice = desc.texture1DArray.mipSlice;
		rtvDesc.Texture1DArray.FirstArraySlice = desc.texture1DArray.firstArraySlice;
		rtvDesc.Texture1DArray.ArraySize = desc.texture1DArray.arraySize;
		break;

	case RenderTargetViewDimension::Texture2D:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = desc.texture2D.mipSlice;
		break;

	case RenderTargetViewDimension::Texture2DArray:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = desc.texture2DArray.mipSlice;
		rtvDesc.Texture2DArray.FirstArraySlice = desc.texture2DArray.firstArraySlice;
		rtvDesc.Texture2DArray.ArraySize = desc.texture2DArray.arraySize;
		break;

	case RenderTargetViewDimension::Texture2DMS:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		break;

	case RenderTargetViewDimension::Texture2DMSArray:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		rtvDesc.Texture2DMSArray.FirstArraySlice = desc.texture2DMSArray.firstArraySlice;
		rtvDesc.Texture2DMSArray.ArraySize = desc.texture2DMSArray.arraySize;
		break;

	case RenderTargetViewDimension::Texture3D:
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		rtvDesc.Texture3D.MipSlice = desc.texture3D.mipSlice;
		rtvDesc.Texture3D.FirstWSlice = desc.texture3D.firstWSlice;
		rtvDesc.Texture3D.WSize = desc.texture3D.wSize;
		break;

	default:
		return {};
	}

	ID3D11Resource* d3dResource = nullptr;

	// Get the native D3D11 resource
	if (const auto buffer = dynamic_cast<D3D11Buffer*>(resource))
	{
		d3dResource = buffer->GetBuffer();
	}
	else if (const auto texture1D = dynamic_cast<D3D11Texture1D*>(resource))
	{
		d3dResource = texture1D->GetTexture();
	}
	else if (const auto texture2D = dynamic_cast<D3D11Texture2D*>(resource))
	{
		d3dResource = texture2D->GetTexture();
	}
	else if (const auto texture3D = dynamic_cast<D3D11Texture3D*>(resource))
	{
		d3dResource = texture3D->GetTexture();
	}

	if (!d3dResource)
		return {};

	ComPtr<ID3D11RenderTargetView> rtv;
	const HRESULT hr = device->CreateRenderTargetView(d3dResource, &rtvDesc, &rtv);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11RenderTargetView>(desc, rtv);
}

PrismObj<ShaderResourceView> D3D11GraphicsDevice::CreateShaderResourceView(Resource* resource, const ShaderResourceViewDesc& desc)
{
	if (!resource)
		return {};

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = ConvertFormat(desc.format);

	switch (desc.dimension)
	{
	case ShaderResourceViewDimension::Buffer:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = desc.buffer.firstElement;
		srvDesc.Buffer.NumElements = desc.buffer.numElements;
		break;

	case ShaderResourceViewDimension::Texture1D:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MostDetailedMip = desc.texture1D.mostDetailedMip;
		srvDesc.Texture1D.MipLevels = desc.texture1D.mipLevels;
		break;

	case ShaderResourceViewDimension::Texture1DArray:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		srvDesc.Texture1DArray.MostDetailedMip = desc.texture1DArray.mostDetailedMip;
		srvDesc.Texture1DArray.MipLevels = desc.texture1DArray.mipLevels;
		srvDesc.Texture1DArray.FirstArraySlice = desc.texture1DArray.firstArraySlice;
		srvDesc.Texture1DArray.ArraySize = desc.texture1DArray.arraySize;
		break;

	case ShaderResourceViewDimension::Texture2D:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = desc.texture2D.mostDetailedMip;
		srvDesc.Texture2D.MipLevels = desc.texture2D.mipLevels;
		break;

	case ShaderResourceViewDimension::Texture2DArray:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = desc.texture2DArray.mostDetailedMip;
		srvDesc.Texture2DArray.MipLevels = desc.texture2DArray.mipLevels;
		srvDesc.Texture2DArray.FirstArraySlice = desc.texture2DArray.firstArraySlice;
		srvDesc.Texture2DArray.ArraySize = desc.texture2DArray.arraySize;
		break;

	case ShaderResourceViewDimension::Texture2DMS:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		break;

	case ShaderResourceViewDimension::Texture2DMSArray:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		srvDesc.Texture2DMSArray.FirstArraySlice = desc.texture2DMSArray.firstArraySlice;
		srvDesc.Texture2DMSArray.ArraySize = desc.texture2DMSArray.arraySize;
		break;

	case ShaderResourceViewDimension::Texture3D:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MostDetailedMip = desc.texture3D.mostDetailedMip;
		srvDesc.Texture3D.MipLevels = desc.texture3D.mipLevels;
		break;

	case ShaderResourceViewDimension::TextureCube:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = desc.textureCube.mostDetailedMip;
		srvDesc.TextureCube.MipLevels = desc.textureCube.mipLevels;
		break;

	case ShaderResourceViewDimension::TextureCubeArray:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		srvDesc.TextureCubeArray.MostDetailedMip = desc.textureCubeArray.mostDetailedMip;
		srvDesc.TextureCubeArray.MipLevels = desc.textureCubeArray.mipLevels;
		srvDesc.TextureCubeArray.First2DArrayFace = desc.textureCubeArray.first2DArrayFace;
		srvDesc.TextureCubeArray.NumCubes = desc.textureCubeArray.numCubes;
		break;

	case ShaderResourceViewDimension::BufferEx:
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = desc.bufferEx.firstElement;
		srvDesc.BufferEx.NumElements = desc.bufferEx.numElements;
		srvDesc.BufferEx.Flags = desc.bufferEx.flags;
		break;

	default:
		return {};
	}

	ID3D11Resource* d3dResource = nullptr;

	if (const auto buffer = dynamic_cast<D3D11Buffer*>(resource))
	{
		d3dResource = buffer->GetBuffer();
	}
	else if (const auto texture1D = dynamic_cast<D3D11Texture1D*>(resource))
	{
		d3dResource = texture1D->GetTexture();
	}
	else if (const auto texture2D = dynamic_cast<D3D11Texture2D*>(resource))
	{
		d3dResource = texture2D->GetTexture();
	}
	else if (const auto texture3D = dynamic_cast<D3D11Texture3D*>(resource))
	{
		d3dResource = texture3D->GetTexture();
	}

	if (!d3dResource)
		return {};

	ComPtr<ID3D11ShaderResourceView> srv;
	const HRESULT hr = device->CreateShaderResourceView(d3dResource, &srvDesc, &srv);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11ShaderResourceView>(desc, srv);
}

PrismObj<DepthStencilView> D3D11GraphicsDevice::CreateDepthStencilView(Resource* resource, const DepthStencilViewDesc& desc)
{
	if (!resource)
		return {};

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = ConvertFormat(desc.format);
	dsvDesc.Flags = 0;

	if ((static_cast<uint32_t>(desc.flags) & static_cast<uint32_t>(DepthStencilViewFlags::ReadOnlyDepth)) != 0)
		dsvDesc.Flags |= D3D11_DSV_READ_ONLY_DEPTH;
	if ((static_cast<uint32_t>(desc.flags) & static_cast<uint32_t>(DepthStencilViewFlags::ReadOnlyStencil)) != 0)
		dsvDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;

	switch (desc.dimension)
	{
	case DepthStencilViewDimension::Texture1D:
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
		dsvDesc.Texture1D.MipSlice = desc.texture1D.mipSlice;
		break;

	case DepthStencilViewDimension::Texture1DArray:
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
		dsvDesc.Texture1DArray.MipSlice = desc.texture1DArray.mipSlice;
		dsvDesc.Texture1DArray.FirstArraySlice = desc.texture1DArray.firstArraySlice;
		dsvDesc.Texture1DArray.ArraySize = desc.texture1DArray.arraySize;
		break;

	case DepthStencilViewDimension::Texture2D:
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = desc.texture2D.mipSlice;
		break;

	case DepthStencilViewDimension::Texture2DArray:
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = desc.texture2DArray.mipSlice;
		dsvDesc.Texture2DArray.FirstArraySlice = desc.texture2DArray.firstArraySlice;
		dsvDesc.Texture2DArray.ArraySize = desc.texture2DArray.arraySize;
		break;

	case DepthStencilViewDimension::Texture2DMS:
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		break;

	case DepthStencilViewDimension::Texture2DMSArray:
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		dsvDesc.Texture2DMSArray.FirstArraySlice = desc.texture2DMSArray.firstArraySlice;
		dsvDesc.Texture2DMSArray.ArraySize = desc.texture2DMSArray.arraySize;
		break;

	default:
		return {};
	}

	ID3D11Resource* d3dResource = nullptr;

	if (const auto texture1D = dynamic_cast<D3D11Texture1D*>(resource))
	{
		d3dResource = texture1D->GetTexture();
	}
	else if (const auto texture2D = dynamic_cast<D3D11Texture2D*>(resource))
	{
		d3dResource = texture2D->GetTexture();
	}

	if (!d3dResource)
		return {};

	ComPtr<ID3D11DepthStencilView> dsv;
	const HRESULT hr = device->CreateDepthStencilView(d3dResource, &dsvDesc, &dsv);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11DepthStencilView>(desc, dsv);
}

PrismObj<UnorderedAccessView> D3D11GraphicsDevice::CreateUnorderedAccessView(Resource* resource, const UnorderedAccessViewDesc& desc)
{
	if (!resource)
		return {};

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = ConvertFormat(desc.format);

	switch (desc.dimension)
	{
	case UnorderedAccessViewDimension::Buffer:
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = desc.buffer.firstElement;
		uavDesc.Buffer.NumElements = desc.buffer.numElements;
		uavDesc.Buffer.Flags = desc.buffer.flags;
		break;

	case UnorderedAccessViewDimension::Texture1D:
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
		uavDesc.Texture1D.MipSlice = desc.texture1D.mipSlice;
		break;

	case UnorderedAccessViewDimension::Texture1DArray:
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
		uavDesc.Texture1DArray.MipSlice = desc.texture1DArray.mipSlice;
		uavDesc.Texture1DArray.FirstArraySlice = desc.texture1DArray.firstArraySlice;
		uavDesc.Texture1DArray.ArraySize = desc.texture1DArray.arraySize;
		break;

	case UnorderedAccessViewDimension::Texture2D:
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = desc.texture2D.mipSlice;
		break;

	case UnorderedAccessViewDimension::Texture2DArray:
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = desc.texture2DArray.mipSlice;
		uavDesc.Texture2DArray.FirstArraySlice = desc.texture2DArray.firstArraySlice;
		uavDesc.Texture2DArray.ArraySize = desc.texture2DArray.arraySize;
		break;

	case UnorderedAccessViewDimension::Texture3D:
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		uavDesc.Texture3D.MipSlice = desc.texture3D.mipSlice;
		uavDesc.Texture3D.FirstWSlice = desc.texture3D.firstWSlice;
		uavDesc.Texture3D.WSize = desc.texture3D.wSize;
		break;

	default:
		return {};
	}

	ID3D11Resource* d3dResource = nullptr;

	if (const auto buffer = dynamic_cast<D3D11Buffer*>(resource))
	{
		d3dResource = buffer->GetBuffer();
	}
	else if (const auto texture1D = dynamic_cast<D3D11Texture1D*>(resource))
	{
		d3dResource = texture1D->GetTexture();
	}
	else if (const auto texture2D = dynamic_cast<D3D11Texture2D*>(resource))
	{
		d3dResource = texture2D->GetTexture();
	}
	else if (const auto texture3D = dynamic_cast<D3D11Texture3D*>(resource))
	{
		d3dResource = texture3D->GetTexture();
	}

	if (!d3dResource)
		return {};

	ComPtr<ID3D11UnorderedAccessView> uav;
	const HRESULT hr = device->CreateUnorderedAccessView(d3dResource, &uavDesc, &uav);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11UnorderedAccessView>(desc, uav);
}

PrismObj<SamplerState> D3D11GraphicsDevice::CreateSamplerState(const SamplerDesc& desc)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	
	// Convert filter
	samplerDesc.Filter = static_cast<D3D11_FILTER>(desc.filter);
	
	// Convert address modes
	samplerDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.addressU);
	samplerDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.addressV);
	samplerDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.addressW);
	
	// Set other parameters
	samplerDesc.MipLODBias = desc.mipLODBias;
	samplerDesc.MaxAnisotropy = desc.maxAnisotropy;
	samplerDesc.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(desc.comparisonFunc);
	
	// Set border color
	samplerDesc.BorderColor[0] = desc.borderColor.r;
	samplerDesc.BorderColor[1] = desc.borderColor.g;
	samplerDesc.BorderColor[2] = desc.borderColor.b;
	samplerDesc.BorderColor[3] = desc.borderColor.a;
	
	// Set LOD range
	samplerDesc.MinLOD = desc.minLOD;
	samplerDesc.MaxLOD = desc.maxLOD;

	ComPtr<ID3D11SamplerState> samplerState;
	const HRESULT hr = device->CreateSamplerState(&samplerDesc, &samplerState);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11SamplerState>(desc, samplerState);
}

PrismObj<GraphicsPipeline> D3D11GraphicsDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
	// TODO: Implement graphics pipeline creation
	// In D3D11, this would involve creating shaders, input layout, etc.
	return {};
}

PrismObj<GraphicsPipelineState> D3D11GraphicsDevice::CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc)
{
	// TODO: Implement graphics pipeline state creation
	// In D3D11, this would involve creating blend state, rasterizer state, depth stencil state
	return {};
}

PrismObj<ComputePipeline> D3D11GraphicsDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
	// TODO: Implement compute pipeline creation
	return {};
}

PrismObj<ComputePipelineState> D3D11GraphicsDevice::CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc)
{
	// TODO: Implement compute pipeline state creation
	return {};
}

HEXA_PRISM_NAMESPACE_END
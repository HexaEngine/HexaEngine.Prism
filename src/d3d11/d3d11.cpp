#include "d3d11/d3d11.hpp"
#include "d3d11/shader_compiler.hpp"
#include <SDL3/SDL.h>

HEXA_PRISM_NAMESPACE_BEGIN

namespace
{
	DXGI_FORMAT ConvertFormat(const Format format)
	{
		return static_cast<DXGI_FORMAT>(format);
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

	UINT ConvertSwapChainFlags(SwapChainFlags flags)
	{
		return static_cast<UINT>(flags);
	}

	DXGI_USAGE ConvertUsageFlags(Usage usage)
	{
		DXGI_USAGE result = 0;
		
		if ((static_cast<uint32_t>(usage) & static_cast<uint32_t>(Usage::BackBuffer)) != 0)
			result |= DXGI_USAGE_BACK_BUFFER;
		if ((static_cast<uint32_t>(usage) & static_cast<uint32_t>(Usage::ReadOnly)) != 0)
			result |= DXGI_USAGE_READ_ONLY;
		if ((static_cast<uint32_t>(usage) & static_cast<uint32_t>(Usage::RenderTargetOutput)) != 0)
			result |= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		if ((static_cast<uint32_t>(usage) & static_cast<uint32_t>(Usage::ShaderInput)) != 0)
			result |= DXGI_USAGE_SHADER_INPUT;
		if ((static_cast<uint32_t>(usage) & static_cast<uint32_t>(Usage::Shared)) != 0)
			result |= DXGI_USAGE_SHARED;
		if ((static_cast<uint32_t>(usage) & static_cast<uint32_t>(Usage::UnorderedAccess)) != 0)
			result |= DXGI_USAGE_UNORDERED_ACCESS;
		
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

void D3D11CommandList::UnsetPipelineState()
{
	if (currentPSO)
	{
		if (auto oldGraphicsState = dynamic_cast<D3D11GraphicsPipelineState*>(currentPSO))
		{
			oldGraphicsState->UnsetState(context.Get());
		}
		if (auto oldComputeState = dynamic_cast<D3D11ComputePipelineState*>(currentPSO))
		{
			oldComputeState->UnsetState(context.Get());
		}
		currentPSO = nullptr;
	}
}

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
	UnsetPipelineState();
	if (state)
	{
		auto d3dState = static_cast<D3D11GraphicsPipelineState*>(state);
		d3dState->SetState(context.Get());
		currentPSO = state;
	}
}

void D3D11CommandList::SetComputePipelineState(ComputePipelineState* state)
{
	UnsetPipelineState();
	if (state)
	{
		auto d3dState = static_cast<D3D11ComputePipelineState*>(state);
		d3dState->SetState(context.Get());
		currentPSO = state;
	}
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
	vp.TopLeftX = viewport.x;
	vp.TopLeftY = viewport.y;
	vp.Width = viewport.width;
	vp.Height = viewport.height;
	vp.MinDepth = viewport.minDepth;
	vp.MaxDepth = viewport.maxDepth;
	context->RSSetViewports(1, &vp);
}

void D3D11CommandList::SetViewports(const uint32_t viewportCount, const Viewport* viewports)
{
	D3D11_VIEWPORT vps[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

	for (uint32_t i = 0; i < viewportCount; i++)
	{
		vps[i].TopLeftX = viewports[i].x;
		vps[i].TopLeftY = viewports[i].y;
		vps[i].Width = viewports[i].width;
		vps[i].Height = viewports[i].height;
		vps[i].MinDepth = viewports[i].minDepth;
		vps[i].MaxDepth = viewports[i].maxDepth;
	}
	context->RSSetViewports(viewportCount, vps);
}

void D3D11CommandList::SetScissorRects(const Rect* rects, const uint32_t rectCount)
{
	D3D11_RECT d3dRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	for (size_t i = 0; i < rectCount; ++i)
	{
		d3dRects[i].left = rects[i].left;
		d3dRects[i].top = rects[i].top;
		d3dRects[i].right = rects[i].right;
		d3dRects[i].bottom = rects[i].bottom;
	}

	context->RSSetScissorRects(rectCount, d3dRects);
}

void D3D11CommandList::SetPrimitiveTopology(PrimitiveTopology topology)
{
	context->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));
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

	const auto d3d11Dsv = static_cast<ID3D11DepthStencilView*>(dsv->GetNativePointer());
	context->ClearDepthStencilView(d3d11Dsv, clearFlags, depth, static_cast<UINT8>(stencil));
}

void D3D11CommandList::ClearUnorderedAccessViewUint(UnorderedAccessView* uav, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	const auto d3d11Uav = static_cast<ID3D11UnorderedAccessView*>(uav->GetNativePointer());
	const UINT clearValues[4] = { r, g, b, a };
	context->ClearUnorderedAccessViewUint(d3d11Uav, clearValues);
}

void D3D11CommandList::ClearView(ResourceView* view, const Color& color, const Rect& rect)
{
	const float clearColor[4] = { color.r, color.g, color.b, color.a };
	
	D3D11_RECT d3dRect;
	d3dRect.left = rect.left;
	d3dRect.top = rect.top;
	d3dRect.right = rect.right;
	d3dRect.bottom = rect.bottom;
	auto* d3dView = static_cast<ID3D11View*>(view->GetNativePointer());
	context->ClearView(d3dView, clearColor, &d3dRect, 1);
}

void D3D11CommandList::CopyResource(Resource* dstResource, Resource* srcResource)
{
	context->CopyResource(static_cast<ID3D11Resource*>(dstResource->GetNativePointer()), static_cast<ID3D11Resource*>(srcResource->GetNativePointer()));
}

void D3D11CommandList::GenerateMips(ShaderResourceView* srv)
{
	context->GenerateMips(static_cast<ID3D11ShaderResourceView*>(srv->GetNativePointer()));
}

void D3D11CommandList::ClearState()
{
	context->ClearState();
}

void D3D11CommandList::Flush()
{
	context->Flush();
}

MappedSubresource D3D11CommandList::Map(Resource* resource, const uint32_t subresource, const MapType mapType, const MapFlags mapFlags)
{
	if (!resource)
	{
		throw std::invalid_argument("Resource cannot be null");
	}

	auto* d3dResource = static_cast<ID3D11Resource*>(resource->GetNativePointer());

	// Convert MapType to D3D11_MAP
	D3D11_MAP d3dMapType;
	switch (mapType)
	{
	case MapType::Read:
		d3dMapType = D3D11_MAP_READ;
		break;
	case MapType::Write:
		d3dMapType = D3D11_MAP_WRITE;
		break;
	case MapType::ReadWrite:
		d3dMapType = D3D11_MAP_READ_WRITE;
		break;
	case MapType::WriteDiscard:
		d3dMapType = D3D11_MAP_WRITE_DISCARD;
		break;
	case MapType::WriteNoOverwrite:
		d3dMapType = D3D11_MAP_WRITE_NO_OVERWRITE;
		break;
	default:
		throw std::invalid_argument("Invalid map type");
	}

	// Convert MapFlags to D3D11 flags
	UINT d3dMapFlags = 0;
	if ((static_cast<uint32_t>(mapFlags) & static_cast<uint32_t>(MapFlags::DoNotWait)) != 0)
	{
		d3dMapFlags |= D3D11_MAP_FLAG_DO_NOT_WAIT;
	}

	// Map the resource
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	const HRESULT hr = context->Map(d3dResource, subresource, d3dMapType, d3dMapFlags, &mappedResource);
	
	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
		{
			// Resource is still being used by GPU
			return MappedSubresource{ nullptr, 0, 0 };
		}
		throw std::runtime_error("Failed to map resource");
	}

	// Convert to our MappedSubresource structure
	MappedSubresource result;
	result.data = mappedResource.pData;
	result.rowPitch = mappedResource.RowPitch;
	result.depthPitch = mappedResource.DepthPitch;
	
	return result;
}

void D3D11CommandList::Unmap(Resource* resource, const uint32_t subresource)
{
	if (!resource)
	{
		throw std::invalid_argument("Resource cannot be null");
	}

	auto* d3dResource = static_cast<ID3D11Resource*>(resource->GetNativePointer());

	context->Unmap(d3dResource, subresource);
}

void D3D11CommandList::BeginQuery(Query* query)
{
	context->Begin(static_cast<D3D11Query*>(query)->GetQuery());
}

void D3D11CommandList::EndQuery(Query* query)
{
	context->End(static_cast<D3D11Query*>(query)->GetQuery());
}

bool D3D11CommandList::QueryGetData(Query* query, void* data, uint32_t size, QueryGetDataFlags flags)
{
	UINT d3dFlags = 0;
	if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(QueryGetDataFlags::DoNotFlush)) != 0)
	{
		d3dFlags |= D3D11_ASYNC_GETDATA_DONOTFLUSH;
	}
	const HRESULT hr = context->GetData(static_cast<D3D11Query*>(query)->GetQuery(), data, size, d3dFlags);
	if (FAILED(hr))
	{
		if (hr == S_FALSE)
		{
			return false;
		}
		throw std::runtime_error("Failed to get query data");
	}
	return true;
}

void D3D11CommandList::BeginEvent(const char* name)
{
	if (!name)
	{
		context->BeginEventInt(L"", 0);
		return;
	}

	// Calculate required buffer size
	const int required = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
	if (required <= 0)
	{
		context->BeginEventInt(L"", 0);
		return;
	}

	constexpr int STACK_BUFFER_SIZE = 1024;
	wchar_t stackBuffer[STACK_BUFFER_SIZE];
	wchar_t* buffer = stackBuffer;
	std::unique_ptr<wchar_t[]> heapBuffer;

	// Use heap allocation if string is too large for stack buffer
	if (required > STACK_BUFFER_SIZE)
	{
		heapBuffer = std::make_unique<wchar_t[]>(required);
		buffer = heapBuffer.get();
	}

	// Convert UTF-8 to UTF-16
	MultiByteToWideChar(CP_UTF8, 0, name, -1, buffer, required);
	context->BeginEventInt(buffer, 0);
}

void D3D11CommandList::EndEvent()
{
	context->EndEvent();
}

// D3D11GraphicsDevice Implementation

bool D3D11GraphicsDevice::Initialize()
{
	HRESULT hr;

	// Create DXGI Factory
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
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

	immediateContext = MakePrismObj<D3D11CommandList>(std::move(ctx), CommandListType::Immediate);

	return true;
}

CommandList* D3D11GraphicsDevice::GetImmediateCommandList()
{
	return immediateContext.Get();
}

PrismObj<Buffer> D3D11GraphicsDevice::CreateBuffer(const BufferDesc& desc, const SubresourceData* initialData)
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

	D3D11_SUBRESOURCE_DATA* subresourceDataPtr = nullptr;
	if (initialData)
	{
		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = initialData->data;
		subresourceData.SysMemPitch = initialData->rowPitch;
		subresourceData.SysMemSlicePitch = initialData->slicePitch;
		subresourceDataPtr = &subresourceData;
	}

	ComPtr<ID3D11Buffer> d3dBuffer;
	const HRESULT hr = device->CreateBuffer(&bufferDesc, subresourceDataPtr, &d3dBuffer);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11Buffer>(desc, std::move(d3dBuffer));
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

	return MakePrismObj<D3D11Texture1D>(desc, std::move(d3dTexture));
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

	return MakePrismObj<D3D11Texture2D>(desc, std::move(d3dTexture));
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

	return MakePrismObj<D3D11Texture3D>(desc, std::move(d3dTexture));
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

	return MakePrismObj<D3D11CommandList>(std::move(deferredContext4), CommandListType::Deferred);
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

	auto* d3dResource = static_cast<ID3D11Resource*>(resource->GetNativePointer());

	if (!d3dResource)
		return {};

	ComPtr<ID3D11RenderTargetView> rtv;
	const HRESULT hr = device->CreateRenderTargetView(d3dResource, &rtvDesc, &rtv);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11RenderTargetView>(desc, std::move(rtv));
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

	auto* d3dResource = static_cast<ID3D11Resource*>(resource->GetNativePointer());

	if (!d3dResource)
		return {};

	ComPtr<ID3D11ShaderResourceView> srv;
	const HRESULT hr = device->CreateShaderResourceView(d3dResource, &srvDesc, &srv);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11ShaderResourceView>(desc, std::move(srv));
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

	auto* d3dResource = static_cast<ID3D11Resource*>(resource->GetNativePointer());

	if (!d3dResource)
		return {};

	ComPtr<ID3D11DepthStencilView> dsv;
	const HRESULT hr = device->CreateDepthStencilView(d3dResource, &dsvDesc, &dsv);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11DepthStencilView>(desc, std::move(dsv));
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

	auto* d3dResource = static_cast<ID3D11Resource*>(resource->GetNativePointer());

	if (!d3dResource)
		return {};

	ComPtr<ID3D11UnorderedAccessView> uav;
	const HRESULT hr = device->CreateUnorderedAccessView(d3dResource, &uavDesc, &uav);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11UnorderedAccessView>(desc, std::move(uav));
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

	return MakePrismObj<D3D11SamplerState>(desc, std::move(samplerState));
}

PrismObj<GraphicsPipeline> D3D11GraphicsDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
	return MakePrismObj<D3D11GraphicsPipeline>(this, desc);
}

PrismObj<GraphicsPipelineState> D3D11GraphicsDevice::CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc)
{
	return MakePrismObj<D3D11GraphicsPipelineState>(PrismObj(static_cast<D3D11GraphicsPipeline*>(pipeline)), desc);
}

PrismObj<ComputePipeline> D3D11GraphicsDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
{
	return MakePrismObj<D3D11ComputePipeline>(this, desc);
}

PrismObj<ComputePipelineState> D3D11GraphicsDevice::CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc)
{
	return MakePrismObj<D3D11ComputePipelineState>(PrismObj(static_cast<D3D11ComputePipeline*>(pipeline)), desc);
}

// D3D11SwapChain Implementation

D3D11SwapChain::D3D11SwapChain(const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc, ComPtr<IDXGISwapChain3>&& swapChain)
	: SwapChain(desc, fullscreenDesc), swapChain(std::move(swapChain))
{
}

void D3D11SwapChain::ResizeBuffers(uint32_t bufferCount, uint32_t width, uint32_t height, Format newFormat, SwapChainFlags swapChainFlags)
{
	DXGI_FORMAT dxgiFormat = ConvertFormat(newFormat);
	UINT flags = ConvertSwapChainFlags(swapChainFlags);

	HRESULT hr = swapChain->ResizeBuffers(bufferCount, width, height, dxgiFormat, flags);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to resize swapchain buffers");
	}

	desc.width = width;
	desc.height = height;
	desc.format = newFormat;
	desc.bufferCount = bufferCount;
	desc.flags = swapChainFlags;
}

PrismObj<Texture2D> D3D11SwapChain::GetBuffer(size_t index)
{
	ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT hr = swapChain->GetBuffer(static_cast<UINT>(index), IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to get swapchain buffer");
	}

	D3D11_TEXTURE2D_DESC d3dDesc;
	backBuffer->GetDesc(&d3dDesc);

	Texture2DDesc desc = {};
	desc.width = d3dDesc.Width;
	desc.height = d3dDesc.Height;
	desc.arraySize = d3dDesc.ArraySize;
	desc.mipLevels = d3dDesc.MipLevels;
	desc.sampleDesc.count = d3dDesc.SampleDesc.Count;
	desc.sampleDesc.quality = d3dDesc.SampleDesc.Quality;
	desc.format = static_cast<Format>(d3dDesc.Format);

	return MakePrismObj<D3D11Texture2D>(desc, std::move(backBuffer));
}

void D3D11SwapChain::Present(uint32_t interval, PresentFlags flags)
{
	UINT presentFlags = 0;
	if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(PresentFlags::DoNotWait)) != 0)
		presentFlags |= DXGI_PRESENT_DO_NOT_WAIT;
	if ((static_cast<uint32_t>(flags) & static_cast<uint32_t>(PresentFlags::AllowTearing)) != 0)
		presentFlags |= DXGI_PRESENT_ALLOW_TEARING;

	HRESULT hr = swapChain->Present(interval, presentFlags);
	if (FAILED(hr) && hr != DXGI_ERROR_WAS_STILL_DRAWING)
	{
		throw std::runtime_error("Failed to present swapchain");
	}
}

namespace
{
	// Helper functions for swap chain creation

	bool CheckSwapChainFormat(ID3D11Device4* device, DXGI_FORMAT format)
	{
		UINT formatSupport = 0;
		HRESULT hr = device->CheckFormatSupport(format, &formatSupport);
		if (FAILED(hr))
		{
			return false;
		}
		return (formatSupport & (D3D11_FORMAT_SUPPORT_DISPLAY | D3D11_FORMAT_SUPPORT_RENDER_TARGET)) != 0;
	}

	DXGI_FORMAT ChooseSwapChainFormat(ID3D11Device4* device, DXGI_FORMAT preferredFormat)
	{
		if (CheckSwapChainFormat(device, preferredFormat))
		{
			return preferredFormat;
		}
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	}

	DXGI_FORMAT AutoChooseSwapChainFormat(ID3D11Device4* device, IDXGIOutput6* output)
	{
		if (!output)
		{
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		}

		DXGI_OUTPUT_DESC1 desc;
		HRESULT hr = output->GetDesc1(&desc);
		if (FAILED(hr))
		{
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		}

		if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
		{
			return ChooseSwapChainFormat(device, DXGI_FORMAT_R10G10B10A2_UNORM);
		}

		if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709)
		{
			return ChooseSwapChainFormat(device, DXGI_FORMAT_B8G8R8A8_UNORM);
		}

		return DXGI_FORMAT_B8G8R8A8_UNORM;
	}

	ComPtr<IDXGIOutput6> GetOutput(IDXGIAdapter3* adapter)
	{
		ComPtr<IDXGIOutput6> selected;
		ComPtr<IDXGIOutput> output;

		for (UINT outputIndex = 0; SUCCEEDED(adapter->EnumOutputs(outputIndex, &output)); outputIndex++)
		{
			ComPtr<IDXGIOutput6> output6;
			HRESULT hr = output.As(&output6);
			if (FAILED(hr))
			{
				output.Reset();
				continue;
			}

			DXGI_OUTPUT_DESC1 desc;
			hr = output6->GetDesc1(&desc);
			if (FAILED(hr))
			{
				output.Reset();
				continue;
			}

			selected = output6;

			if (desc.DesktopCoordinates.top == 0 && desc.DesktopCoordinates.left == 0)
			{
				break;
			}

			output.Reset();
		}

		return selected;
	}
}

PrismObj<SwapChain> D3D11GraphicsDevice::CreateSwapChain(void* windowHandle, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc)
{
	SDL_Window* sdlWindow = static_cast<SDL_Window*>(windowHandle);
	HWND hwnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = desc.width;
	swapChainDesc.Height = desc.height;
	swapChainDesc.Format = ConvertFormat(desc.format);
	swapChainDesc.Stereo = desc.stereo ? TRUE : FALSE;
	swapChainDesc.SampleDesc.Count = desc.sampleDesc.count;
	swapChainDesc.SampleDesc.Quality = desc.sampleDesc.quality;
	swapChainDesc.BufferUsage = ConvertUsageFlags(desc.bufferUsage);
	swapChainDesc.BufferCount = desc.bufferCount;
	swapChainDesc.Scaling = static_cast<DXGI_SCALING>(desc.scaling);
	swapChainDesc.SwapEffect = static_cast<DXGI_SWAP_EFFECT>(desc.swapEffect);
	swapChainDesc.AlphaMode = static_cast<DXGI_ALPHA_MODE>(desc.alphaMode);
	swapChainDesc.Flags = ConvertSwapChainFlags(desc.flags);

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDescDXGI = {};
	fullscreenDescDXGI.RefreshRate.Numerator = fullscreenDesc.refreshRate.numerator;
	fullscreenDescDXGI.RefreshRate.Denominator = fullscreenDesc.refreshRate.denominator;
	fullscreenDescDXGI.ScanlineOrdering = static_cast<DXGI_MODE_SCANLINE_ORDER>(fullscreenDesc.scanlineOrdering);
	fullscreenDescDXGI.Scaling = static_cast<DXGI_MODE_SCALING>(fullscreenDesc.scaling);
	fullscreenDescDXGI.Windowed = fullscreenDesc.windowed ? TRUE : FALSE;

	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = factory->CreateSwapChainForHwnd(
		device.Get(),
		hwnd,
		&swapChainDesc,
		&fullscreenDescDXGI,
		nullptr,
		&swapChain1
	);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create swapchain");
	}

	ComPtr<IDXGISwapChain3> swapChain3;
	hr = swapChain1.As(&swapChain3);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to query IDXGISwapChain3");
	}

	return MakePrismObj<D3D11SwapChain>(desc, fullscreenDesc, std::move(swapChain3));
}

PrismObj<SwapChain> D3D11GraphicsDevice::CreateSwapChain(void* windowHandle)
{
	SDL_Window* sdlWindow = static_cast<SDL_Window*>(windowHandle);
	HWND hwnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

	int width = 0;
	int height = 0;
	SDL_GetWindowSize(sdlWindow, &width, &height);

	ComPtr<IDXGIOutput6> output = GetOutput(adapter.Get());
	DXGI_FORMAT dxgiFormat = AutoChooseSwapChainFormat(device.Get(), output.Get());

	SwapChainDesc desc = {};
	desc.width = static_cast<uint32_t>(width);
	desc.height = static_cast<uint32_t>(height);
	desc.format = static_cast<Format>(dxgiFormat);
	desc.stereo = false;
	desc.sampleDesc = { 1, 0 };
	desc.bufferUsage = Usage::RenderTargetOutput;
	desc.bufferCount = 2;
	desc.scaling = Scaling::Stretch;
	desc.swapEffect = SwapEffect::FlipSequential;
	desc.alphaMode = AlphaMode::Unspecified;
	desc.flags = static_cast<SwapChainFlags>(static_cast<uint32_t>(SwapChainFlags::AllowModeSwitch) | static_cast<uint32_t>(SwapChainFlags::AllowTearing));

	SwapChainFullscreenDesc fullscreenDesc = {};
	fullscreenDesc.windowed = true;
	fullscreenDesc.refreshRate = { 0, 1 };
	fullscreenDesc.scaling = Scaling::None;
	fullscreenDesc.scanlineOrdering = ScanlineOrder::Unspecified;

	return CreateSwapChain(windowHandle, desc, fullscreenDesc);
}

PrismObj<Query> D3D11GraphicsDevice::CreateQuery(const QueryDesc& desc)
{
	D3D11_QUERY_DESC1 d3dDesc;
	d3dDesc.Query = static_cast<D3D11_QUERY>(desc.type);
	d3dDesc.ContextType = static_cast<D3D11_CONTEXT_TYPE>(desc.contextType);
	d3dDesc.MiscFlags = static_cast<D3D11_QUERY_MISC_FLAG>(desc.miscFlags);

	ComPtr<ID3D11Query1> query;
	HRESULT hr = device->CreateQuery1(&d3dDesc, &query);
	if (FAILED(hr))
	{
		return {};
	}

	return MakePrismObj<D3D11Query>(desc, std::move(query));
}

HEXA_PRISM_NAMESPACE_END

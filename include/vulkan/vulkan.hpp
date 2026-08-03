#pragma once
#include "common.hpp"
#include <vulkan/vulkan_core.h>

HEXA_PRISM_NAMESPACE_BEGIN

class VulkanDevice;

class VulkanDeviceChild
{
protected:
	VulkanDevice* device;
	VulkanDeviceChild(VulkanDevice* device) : device(device) {}

public:
	PrismDevice* GetDevice();
};

class VulkanRenderTargetView : public VulkanDeviceChild, public RenderTargetView
{
	VkImageView imageView;
public:
	VulkanRenderTargetView(VulkanDevice* device, VkImageView imageView, const RenderTargetViewDesc& desc) : VulkanDeviceChild(device), RenderTargetView(desc), imageView(imageView)
	{
	}	
	
	VkImageView GetImageView() const { return imageView; }
};

class VulkanDepthStencilView : public VulkanDeviceChild, public DepthStencilView
{
	VkImageView imageView;
public:
	VulkanDepthStencilView(VulkanDevice* device, VkImageView imageView, const DepthStencilViewDesc& desc) : VulkanDeviceChild(device), DepthStencilView(desc), imageView(imageView)
	{
	}

	VkImageView GetImageView() const { return imageView; }
};

class VulkanSwapChain : public SwapChain
{
	VkSwapchainKHR swapchain;
public:
	VulkanSwapChain(VkSwapchainKHR swapchain, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc)
		: SwapChain(desc, fullscreenDesc), swapchain(swapchain)
	{
	}
};

class VulkanFence : public Fence
{
	VulkanDevice* device;
	VkFence fence;
public:
	VulkanFence(VulkanDevice* device, VkFence fence) : device(device), fence(fence) {}
	~VulkanFence();

	VkFence GetFence() const noexcept { return fence; }
	void* GetNativePointer() const noexcept override { return fence; }
	PrismDevice* GetDevice() const noexcept override { return device; }
};

class VulkanCommandQueue : public CommandQueue
{
	VulkanDevice* device;
	VkQueue queue;

public:
	VulkanCommandQueue(const CommandQueueDesc& desc, VulkanDevice* device, VkQueue queue) : CommandQueue(desc), device(device), queue(queue) {}
	void Submit(CommandList** lists, uint32_t count, Fence* fence) override;
	void WaitIdle() override;

	VkQueue GetQueue() const noexcept { return queue; }
	void* GetNativePointer() const noexcept override { return queue; }
	PrismDevice* GetDevice() const noexcept override { return device; }
};

class VulkanCommandAllocator : public CommandAllocator
{
	VulkanDevice* device;
	VkCommandPool commandPool;

public:
	VulkanCommandAllocator(const CommandAllocatorDesc& desc, VulkanDevice* device, VkCommandPool commandPool) : CommandAllocator(desc), device(device), commandPool(commandPool) {}
	~VulkanCommandAllocator();
	bool Reset() override;

	VkCommandPool GetCommandPool() const noexcept { return commandPool; }
	void* GetNativePointer() const noexcept override { return commandPool; }
	PrismDevice* GetDevice() const noexcept override { return device; }
};

static constexpr uint32_t VkMaxSimultaneousRenderTargets = 8;

class VulkanCommandList : public CommandList
{
	struct CommandListState
	{
		VulkanRenderTargetView* rtvs[VkMaxSimultaneousRenderTargets];
		VulkanDepthStencilView* dsv;
		PipelineState* state;
		Viewport viewports[VkMaxSimultaneousRenderTargets];
		Rect scissors[VkMaxSimultaneousRenderTargets];
		uint32_t scissorCount = 0;

		Buffer* vertexBuffers[16];
		uint32_t vertexStrides[16];
		uint32_t vertexOffsets[16];
		uint32_t vertexBufferCount = 0;

		Buffer* indexBuffer = nullptr;
		uint32_t indexOffset = 0;
		Format indexFormat = Format::Unknown;
	};

	VulkanDevice* device;
	VkCommandBuffer commandBuffer;
	CommandListType type;
	CommandListState state;
	bool drawing;
	void EnsureDrawBegin();
	void EnsureDrawEnd();
public:
	VulkanCommandList(const CommandListDesc& desc, VulkanDevice* device, VkCommandBuffer commandBuffer) : CommandList(desc), device(device), commandBuffer(commandBuffer), type(type) {}
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
	void SetPrimitiveTopology(PrimitiveTopology topology) override;
	void SetScissorRects(const Rect* rects, uint32_t rectCount) override;
	void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset) override;
	void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, int32_t vertexOffset, uint32_t instanceOffset) override;
	void DrawIndexedInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs) override;
	void DrawInstancedIndirect(Buffer* bufferForArgs, uint32_t alignedByteOffsetForArgs) override;
	void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;
	void DispatchIndirect(Buffer* dispatchArgs, uint32_t offset) override;
	void ExecuteCommandList(CommandList* commandList) override;
	void ClearRenderTargetView(RenderTargetView* rtv, const Color& color) override;
	void ClearDepthStencilView(DepthStencilView* dsv, DepthStencilViewClearFlags flags, float depth, char stencil) override;
	void ClearUnorderedAccessViewUint(UnorderedAccessView* uav, uint32_t r, uint32_t g, uint32_t b, uint32_t a) override;
	void ClearView(ResourceView* view, const Color& color, const Rect& rect) override;
	void CopyResource(Resource* dstResource, Resource* srcResource) override;
	void GenerateMips(ShaderResourceView* srv) override;
	void ClearState() override;
	void Flush() override;
	MappedSubresource Map(Resource* resource, uint32_t subresource, MapType mapType, MapFlags mapFlags) override;
	void Unmap(Resource* resource, uint32_t subresource) override;
	void BeginQuery(Query* query) override;
	void EndQuery(Query* query) override;
	bool QueryGetData(Query* query, void* data, uint32_t size, QueryGetDataFlags flags = QueryGetDataFlags::None) override;

	void BeginEvent(const char* name) override;
	void EndEvent() override;

	VkCommandBuffer GetCommandBuffer() const noexcept { return commandBuffer; }
};

struct QueueFamilyIndices 
{
	static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);
    uint32_t graphics = InvalidIndex;
    uint32_t compute = InvalidIndex;
    uint32_t transfer = InvalidIndex;
};

class VulkanDevice : public PrismDevice
{
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
	QueueFamilyIndices queueIndicies;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue computeQueue = VK_NULL_HANDLE;
	VkQueue transferQueue = VK_NULL_HANDLE;
    
    bool FindQueueFamily(QueueFamilyIndices& indices) const;
    bool CreateLogicalDevice();
public:
    VulkanDevice() = default;
    bool Initialize(const DeviceDesc& desc);
	CommandQueue* GetCommandQueue(uint32_t index) override;
	PrismObj<CommandAllocator> CreateCommandAllocator(const CommandAllocatorDesc& desc) override;
	PrismObj<CommandList> CreateCommandList(const CommandListDesc& desc) override;
	PrismObj<Buffer> CreateBuffer(const BufferDesc& desc, const SubresourceData* initialData = nullptr) override;
	PrismObj<Texture1D> CreateTexture1D(const Texture1DDesc& desc) override;
	PrismObj<Texture2D> CreateTexture2D(const Texture2DDesc& desc) override;
	PrismObj<Texture3D> CreateTexture3D(const Texture3DDesc& desc) override;
	PrismObj<RenderTargetView> CreateRenderTargetView(Resource* resource, const RenderTargetViewDesc& desc) override;
	PrismObj<ShaderResourceView> CreateShaderResourceView(Resource* resource, const ShaderResourceViewDesc& desc) override;
	PrismObj<DepthStencilView> CreateDepthStencilView(Resource* resource, const DepthStencilViewDesc& desc) override;
	PrismObj<UnorderedAccessView> CreateUnorderedAccessView(Resource* resource, const UnorderedAccessViewDesc& desc) override;
	PrismObj<SamplerState> CreateSamplerState(const SamplerDesc& desc) override;
	PrismObj<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
	PrismObj<GraphicsPipelineState> CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc) override;
	PrismObj<ComputePipeline> CreateComputePipeline(const ComputePipelineDesc& desc) override;
	PrismObj<ComputePipelineState> CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc) override;
	PrismObj<SwapChain> CreateSwapChain(void* windowHandle, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc) override;
	PrismObj<SwapChain> CreateSwapChain(void* windowHandle) override;
	PrismObj<Query> CreateQuery(const QueryDesc& desc) override;

    const VkInstance& GetInstance() const { return instance; }
    const VkPhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }
    const VkDevice& GetDevice() const { return device; }
	const VkAllocationCallbacks* GetAllocationCallbacks() const { return nullptr; }
};

HEXA_PRISM_NAMESPACE_END
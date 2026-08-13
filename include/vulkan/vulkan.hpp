#pragma once
#include "common.hpp"
#include "shader_compiler.hpp"
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

class VulkanBuffer : public VulkanDeviceChild, public Buffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
public:
	VulkanBuffer(VulkanDevice* device, const BufferDesc& desc, VkBuffer buffer, VmaAllocation allocation)
		: VulkanDeviceChild(device), Buffer(desc), buffer(buffer), allocation(allocation)
	{
	}
	~VulkanBuffer() override;

	VkBuffer GetBuffer() const { return buffer; }
	VmaAllocation GetAllocation() const { return allocation; }
	void* GetNativePointer() const noexcept override { return buffer; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanTexture1D : public VulkanDeviceChild, public Texture1D
{
	VkImage image;
	VmaAllocation allocation;
	VkImageLayout currentLayout;
public:
	VulkanTexture1D(VulkanDevice* device, const Texture1DDesc& desc, VkImage image, VmaAllocation allocation)
		: VulkanDeviceChild(device), Texture1D(desc), image(image), allocation(allocation), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
	}
	~VulkanTexture1D() override;

	VkImage GetImage() const { return image; }
	VmaAllocation GetAllocation() const { return allocation; }
	VkImageLayout GetCurrentLayout() const { return currentLayout; }
	void SetCurrentLayout(VkImageLayout layout) { currentLayout = layout; }
	void* GetNativePointer() const noexcept override { return image; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanTexture2D : public VulkanDeviceChild, public Texture2D
{
	VkImage image;
	VmaAllocation allocation;
	VkImageLayout currentLayout;
public:
	VulkanTexture2D(VulkanDevice* device, const Texture2DDesc& desc, VkImage image, VmaAllocation allocation)
		: VulkanDeviceChild(device), Texture2D(desc), image(image), allocation(allocation), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
	}
	~VulkanTexture2D() override;

	VkImage GetImage() const { return image; }
	VmaAllocation GetAllocation() const { return allocation; }
	VkImageLayout GetCurrentLayout() const { return currentLayout; }
	void SetCurrentLayout(VkImageLayout layout) { currentLayout = layout; }
	void* GetNativePointer() const noexcept override { return image; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanTexture3D : public VulkanDeviceChild, public Texture3D
{
	VkImage image;
	VmaAllocation allocation;
	VkImageLayout currentLayout;
public:
	VulkanTexture3D(VulkanDevice* device, const Texture3DDesc& desc, VkImage image, VmaAllocation allocation)
		: VulkanDeviceChild(device), Texture3D(desc), image(image), allocation(allocation), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
	}
	~VulkanTexture3D() override;

	VkImage GetImage() const { return image; }
	VmaAllocation GetAllocation() const { return allocation; }
	VkImageLayout GetCurrentLayout() const { return currentLayout; }
	void SetCurrentLayout(VkImageLayout layout) { currentLayout = layout; }
	void* GetNativePointer() const noexcept override { return image; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanRenderTargetView : public VulkanDeviceChild, public RenderTargetView
{
	VkImageView imageView;
	PrismObj<Resource> resource;
public:
	VulkanRenderTargetView(VulkanDevice* device, VkImageView imageView, Resource* resource, const RenderTargetViewDesc& desc) : VulkanDeviceChild(device), RenderTargetView(desc), imageView(imageView), resource(resource)
	{
	}
	~VulkanRenderTargetView() override;

	VkImageView GetImageView() const { return imageView; }
	Resource* GetResource() const { return resource.Get(); }
	void* GetNativePointer() const noexcept override { return imageView; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanDepthStencilView : public VulkanDeviceChild, public DepthStencilView
{
	VkImageView imageView;
	PrismObj<Resource> resource;
public:
	VulkanDepthStencilView(VulkanDevice* device, VkImageView imageView, Resource* resource, const DepthStencilViewDesc& desc) : VulkanDeviceChild(device), DepthStencilView(desc), imageView(imageView), resource(resource)
	{
	}
	~VulkanDepthStencilView() override;

	VkImageView GetImageView() const { return imageView; }
	Resource* GetResource() const { return resource.Get(); }
	void* GetNativePointer() const noexcept override { return imageView; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanShaderResourceView : public VulkanDeviceChild, public ShaderResourceView
{
	VkImageView imageView;
	PrismObj<Resource> resource;
public:
	VulkanShaderResourceView(VulkanDevice* device, VkImageView imageView, Resource* resource, const ShaderResourceViewDesc& desc) : VulkanDeviceChild(device), ShaderResourceView(desc), imageView(imageView), resource(resource)
	{
	}
	~VulkanShaderResourceView() override;

	VkImageView GetImageView() const { return imageView; }
	Resource* GetResource() const { return resource.Get(); }
	void* GetNativePointer() const noexcept override { return imageView; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanUnorderedAccessView : public VulkanDeviceChild, public UnorderedAccessView
{
	VkImageView imageView;
	PrismObj<Resource> resource;
public:
	VulkanUnorderedAccessView(VulkanDevice* device, VkImageView imageView, Resource* resource, const UnorderedAccessViewDesc& desc) : VulkanDeviceChild(device), UnorderedAccessView(desc), imageView(imageView), resource(resource)
	{
	}
	~VulkanUnorderedAccessView() override;

	VkImageView GetImageView() const { return imageView; }
	Resource* GetResource() const { return resource.Get(); }
	void* GetNativePointer() const noexcept override { return imageView; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanSamplerState : public VulkanDeviceChild, public SamplerState
{
	VkSampler sampler;
public:
	VulkanSamplerState(VulkanDevice* device, VkSampler sampler, const SamplerDesc& desc) : VulkanDeviceChild(device), SamplerState(desc), sampler(sampler)
	{
	}
	~VulkanSamplerState() override;

	VkSampler GetSampler() const { return sampler; }
	void* GetNativePointer() const noexcept override { return sampler; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanSwapChain : public SwapChain
{
	VulkanDevice* device;
	void* windowHandle;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkFormat vkFormat;
	VkFence acquireFence;
	std::vector<PrismObj<VulkanTexture2D>> buffers;
	uint32_t currentImageIndex;
	bool imageAcquired;

	bool CreateOrResizeSwapchain(uint32_t width, uint32_t height, VkFormat format, uint32_t bufferCount);
	void DestroySwapchainResources();

public:
	VulkanSwapChain(VulkanDevice* device, void* windowHandle, VkSurfaceKHR surface, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc);
	~VulkanSwapChain() override;

	void ResizeBuffers(uint32_t bufferCount, uint32_t width, uint32_t height, Format newFormat, SwapChainFlags swapChainFlags) override;
	PrismObj<Texture2D> GetBuffer(size_t index) override;
	void Present(uint32_t interval, PresentFlags flags) override;

	VkSwapchainKHR GetSwapchain() const noexcept { return swapchain; }
};

class VulkanResourceBindingList;

class VulkanGraphicsPipeline final : public VulkanDeviceChild, public GraphicsPipeline
{
	VkShaderModule vertexModule = VK_NULL_HANDLE;
	VkShaderModule hullModule = VK_NULL_HANDLE;
	VkShaderModule domainModule = VK_NULL_HANDLE;
	VkShaderModule geometryModule = VK_NULL_HANDLE;
	VkShaderModule pixelModule = VK_NULL_HANDLE;
	PrismObj<Blob> vertexSpirv, hullSpirv, domainSpirv, geometrySpirv, pixelSpirv;
	bool valid = false;

	void Compile();

public:
	VulkanGraphicsPipeline(VulkanDevice* device, const GraphicsPipelineDesc& desc);
	~VulkanGraphicsPipeline() override;

	bool IsValid() const noexcept { return valid; }
	VkShaderModule GetVertexModule() const noexcept { return vertexModule; }
	VkShaderModule GetHullModule() const noexcept { return hullModule; }
	VkShaderModule GetDomainModule() const noexcept { return domainModule; }
	VkShaderModule GetGeometryModule() const noexcept { return geometryModule; }
	VkShaderModule GetPixelModule() const noexcept { return pixelModule; }

	const PrismObj<Blob>& GetVertexSpirv() const noexcept { return vertexSpirv; }
	const PrismObj<Blob>& GetHullSpirv() const noexcept { return hullSpirv; }
	const PrismObj<Blob>& GetDomainSpirv() const noexcept { return domainSpirv; }
	const PrismObj<Blob>& GetGeometrySpirv() const noexcept { return geometrySpirv; }
	const PrismObj<Blob>& GetPixelSpirv() const noexcept { return pixelSpirv; }
};

class VulkanComputePipeline final : public VulkanDeviceChild, public ComputePipeline
{
	VkShaderModule computeModule = VK_NULL_HANDLE;
	PrismObj<Blob> computeSpirv;
	bool valid = false;

	void Compile();

public:
	VulkanComputePipeline(VulkanDevice* device, const ComputePipelineDesc& desc);
	~VulkanComputePipeline() override;

	bool IsValid() const noexcept { return valid; }
	VkShaderModule GetComputeModule() const noexcept { return computeModule; }
	const PrismObj<Blob>& GetComputeSpirv() const noexcept { return computeSpirv; }
};

class VulkanGraphicsPipelineState final : public VulkanDeviceChild, public GraphicsPipelineState
{
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline vkPipeline = VK_NULL_HANDLE;
	std::unique_ptr<VulkanResourceBindingList> bindingList;
	bool valid = false;

	void Create();

public:
	VulkanGraphicsPipelineState(VulkanDevice* device, const PrismObj<GraphicsPipeline>& pipeline, const GraphicsPipelineStateDesc& desc);
	~VulkanGraphicsPipelineState() override;

	ResourceBindingList& GetBindings() override;

	bool IsValid() const noexcept { return valid; }
	VkPipeline GetVkPipeline() const noexcept { return vkPipeline; }
	VkPipelineLayout GetLayout() const noexcept { return layout; }
};

class VulkanComputePipelineState final : public VulkanDeviceChild, public ComputePipelineState
{
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline vkPipeline = VK_NULL_HANDLE;
	std::unique_ptr<VulkanResourceBindingList> bindingList;
	bool valid = false;

	void Create();

public:
	VulkanComputePipelineState(VulkanDevice* device, const PrismObj<ComputePipeline>& pipeline, const ComputePipelineStateDesc& desc);
	~VulkanComputePipelineState() override;

	ResourceBindingList& GetBindings() override;

	bool IsValid() const noexcept { return valid; }
	VkPipeline GetVkPipeline() const noexcept { return vkPipeline; }
	VkPipelineLayout GetLayout() const noexcept { return layout; }
};

class VulkanFence : public Fence
{
	VulkanDevice* device;
	VkSemaphore semaphore;
public:
	VulkanFence(VulkanDevice* device, VkSemaphore semaphore) : device(device), semaphore(semaphore) {}
	~VulkanFence();

	void Signal(uint64_t value) override;
	uint64_t GetCompletedValue() const override;
	bool Wait(uint64_t value, uint64_t timeoutNs = UINT64_MAX) override;

	VkSemaphore GetSemaphore() const noexcept { return semaphore; }
	void* GetNativePointer() const noexcept override { return semaphore; }
	PrismDevice* GetDevice() const noexcept override;
};

class VulkanCommandQueue : public CommandQueue
{
	VulkanDevice* device;
	VkQueue queue;

public:
	VulkanCommandQueue(const CommandQueueDesc& desc, VulkanDevice* device, VkQueue queue) : CommandQueue(desc), device(device), queue(queue) {}
	void Submit(CommandList** lists, uint32_t count, Fence* fence = nullptr, uint64_t signalValue = 0) override;
	void WaitIdle() override;

	VkQueue GetQueue() const noexcept { return queue; }
	void* GetNativePointer() const noexcept override { return queue; }
	PrismDevice* GetDevice() const noexcept override;
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
	PrismDevice* GetDevice() const noexcept override;
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
	VulkanCommandList(const CommandListDesc& desc, VulkanDevice* device, VkCommandBuffer commandBuffer) : CommandList(desc), device(device), commandBuffer(commandBuffer), type(desc.type), drawing(false) {}
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
	void* GetNativePointer() const noexcept override { return commandBuffer; }
	PrismDevice* GetDevice() const noexcept override;
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
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	PrismObj<VulkanCommandQueue> graphicsCommandQueue;

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
	PrismObj<Fence> CreateFence(uint64_t initialValue) override;

    const VkInstance& GetInstance() const { return instance; }
    const VkPhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }
    const VkDevice& GetDevice() const { return device; }
	const VkAllocationCallbacks* GetAllocationCallbacks() const { return nullptr; }
	const VkCommandPool& GetCommandPool() const { return commandPool; }
	const VmaAllocator& GetAllocator() const { return allocator; }
	const VkDescriptorPool& GetDescriptorPool() const { return descriptorPool; }
	const VkQueue& GetGraphicsQueue() const { return graphicsQueue; }
};

HEXA_PRISM_NAMESPACE_END
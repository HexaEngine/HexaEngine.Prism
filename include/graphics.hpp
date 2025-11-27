#pragma once
#include "common.hpp"
#include <cstdint>

HEXA_PRISM_NAMESPACE_BEGIN
	struct Viewport
	{
		float X, Y;
		float Width, Height;
		float MinDepth, MaxDepth;
	};

	struct Color
	{
		float r, g, b, a;
	};

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
		constexpr PrismObj() : ptr(nullptr)
		{
		}

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

		template <typename U>
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


	template <typename T, typename... TArgs>
	[[nodiscard]] inline PrismObj<T> MakePrismObj(TArgs&&... args)
	{
		return PrismObj<T>(new T(std::forward<TArgs>(args)...));
	}

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
		Texture1DDesc desc;

	protected:
		Texture1D(const Texture1DDesc& desc) : desc(desc)
		{
		}

	public:
		const Texture1DDesc& GetDesc() const { return desc; }
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
		Texture2DDesc desc;

	protected:
		Texture2D(const Texture2DDesc& desc) : desc(desc)
		{
		}

	public:
		const Texture2DDesc& GetDesc() const { return desc; }
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
		Texture3DDesc desc;

	protected:
		Texture3D(const Texture3DDesc& desc) : desc(desc)
		{
		}

	public:
		const Texture3DDesc& GetDesc() const { return desc; }
	};

	enum class BufferType
	{
		Default,
		ConstantBuffer,
		VertexBuffer,
		IndexBuffer,
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

	protected:
		Buffer(const BufferDesc& desc) : desc(desc)
		{
		}

	public:
		const BufferDesc& GetDesc() const { return desc; }
	};

	class ShaderSource : public PrismObject
	{
	public:
		virtual const char* GetIdentifier() = 0;
		virtual void GetData(uint8_t*& data, size_t& dataLength) = 0;
	};

	class PipelineState : public PrismObject
	{
	};

	struct GraphicsPipelineDesc
	{
		PrismObj<ShaderSource> vertexShader;
		PrismObj<ShaderSource> hullShader;
		PrismObj<ShaderSource> domainShader;
		PrismObj<ShaderSource> geometryShader;
		PrismObj<ShaderSource> pixelShader;
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
		PrismObj<ShaderSource> computeShader;
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

	// RenderTargetView structures
	enum class RenderTargetViewDimension
	{
		Buffer,
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture2DMS,
		Texture2DMSArray,
		Texture3D,
	};

	struct BufferRTV
	{
		union
		{
			uint32_t firstElement;
			uint32_t elementOffset;
		};

		union
		{
			uint32_t numElements;
			uint32_t elementWidth;
		};
	};

	struct Tex1DRTV
	{
		uint32_t mipSlice;
	};

	struct Tex1DArrayRTV
	{
		uint32_t mipSlice;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DRTV
	{
		uint32_t mipSlice;
	};

	struct Tex2DArrayRTV
	{
		uint32_t mipSlice;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DMSRTV
	{
		uint32_t unusedField_NothingToDefine;
	};

	struct Tex2DMSArrayRTV
	{
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex3DRTV
	{
		uint32_t mipSlice;
		uint32_t firstWSlice;
		uint32_t wSize;
	};

	struct RenderTargetViewDesc
	{
		RenderTargetViewDimension dimension;
		Format format;

		union
		{
			BufferRTV buffer;
			Tex1DRTV texture1D;
			Tex1DArrayRTV texture1DArray;
			Tex2DRTV texture2D;
			Tex2DArrayRTV texture2DArray;
			Tex2DMSRTV texture2DMS;
			Tex2DMSArrayRTV texture2DMSArray;
			Tex3DRTV texture3D;
		};
	};

	class RenderTargetView : public ResourceView
	{
		RenderTargetViewDesc desc;

	protected:
		explicit RenderTargetView(const RenderTargetViewDesc& desc) : desc(desc)
		{
		}

	public:
		const RenderTargetViewDesc& GetDesc() const { return desc; }
	};

	// ShaderResourceView structures
	enum class ShaderResourceViewDimension
	{
		Unknown,
		Buffer,
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture2DMS,
		Texture2DMSArray,
		Texture3D,
		TextureCube,
		TextureCubeArray,
		BufferEx,
	};

	struct BufferSRV
	{
		union
		{
			uint32_t firstElement;
			uint32_t elementOffset;
		};

		union
		{
			uint32_t numElements;
			uint32_t elementWidth;
		};
	};

	struct Tex1DSRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
	};

	struct Tex1DArraySRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DSRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
	};

	struct Tex2DArraySRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DMSSRV
	{
		uint32_t unusedField_NothingToDefine;
	};

	struct Tex2DMSArraySRV
	{
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex3DSRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
	};

	struct TexCubeSRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
	};

	struct TexCubeArraySRV
	{
		uint32_t mostDetailedMip;
		uint32_t mipLevels;
		uint32_t first2DArrayFace;
		uint32_t numCubes;
	};

	struct BufferExSRV
	{
		uint32_t firstElement;
		uint32_t numElements;
		uint32_t flags;
	};

	struct ShaderResourceViewDesc
	{
		ShaderResourceViewDimension dimension;
		Format format;

		union
		{
			BufferSRV buffer;
			Tex1DSRV texture1D;
			Tex1DArraySRV texture1DArray;
			Tex2DSRV texture2D;
			Tex2DArraySRV texture2DArray;
			Tex2DMSSRV texture2DMS;
			Tex2DMSArraySRV texture2DMSArray;
			Tex3DSRV texture3D;
			TexCubeSRV textureCube;
			TexCubeArraySRV textureCubeArray;
			BufferExSRV bufferEx;
		};
	};

	class ShaderResourceView : public ResourceView
	{
		ShaderResourceViewDesc desc;

	protected:
		explicit ShaderResourceView(const ShaderResourceViewDesc& desc) : desc(desc)
		{
		}

	public:
		const ShaderResourceViewDesc& GetDesc() const { return desc; }
	};

	// DepthStencilView structures
	enum class DepthStencilViewDimension
	{
		Unknown,
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture2DMS,
		Texture2DMSArray,
	};

	enum class DepthStencilViewFlags
	{
		None = 0,
		ReadOnlyDepth = 1 << 0,
		ReadOnlyStencil = 1 << 1,
	};

	struct Tex1DDSV
	{
		uint32_t mipSlice;
	};

	struct Tex1DArrayDSV
	{
		uint32_t mipSlice;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DDSV
	{
		uint32_t mipSlice;
	};

	struct Tex2DArrayDSV
	{
		uint32_t mipSlice;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DMSDSV
	{
		uint32_t unusedField_NothingToDefine;
	};

	struct Tex2DMSArrayDSV
	{
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct DepthStencilViewDesc
	{
		DepthStencilViewDimension dimension;
		Format format;
		DepthStencilViewFlags flags;

		union
		{
			Tex1DDSV texture1D;
			Tex1DArrayDSV texture1DArray;
			Tex2DDSV texture2D;
			Tex2DArrayDSV texture2DArray;
			Tex2DMSDSV texture2DMS;
			Tex2DMSArrayDSV texture2DMSArray;
		};
	};

	class DepthStencilView : public ResourceView
	{
		DepthStencilViewDesc desc;

	protected:
		explicit DepthStencilView(const DepthStencilViewDesc& desc) : desc(desc)
		{
		}

	public:
		const DepthStencilViewDesc& GetDesc() const { return desc; }
	};

	// UnorderedAccessView structures
	enum class UnorderedAccessViewDimension
	{
		Unknown,
		Buffer,
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture3D,
	};

	struct BufferUAV
	{
		uint32_t firstElement;
		uint32_t numElements;
		uint32_t flags;
	};

	struct Tex1DUAV
	{
		uint32_t mipSlice;
	};

	struct Tex1DArrayUAV
	{
		uint32_t mipSlice;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex2DUAV
	{
		uint32_t mipSlice;
	};

	struct Tex2DArrayUAV
	{
		uint32_t mipSlice;
		uint32_t firstArraySlice;
		uint32_t arraySize;
	};

	struct Tex3DUAV
	{
		uint32_t mipSlice;
		uint32_t firstWSlice;
		uint32_t wSize;
	};

	struct UnorderedAccessViewDesc
	{
		UnorderedAccessViewDimension dimension;
		Format format;

		union
		{
			BufferUAV buffer;
			Tex1DUAV texture1D;
			Tex1DArrayUAV texture1DArray;
			Tex2DUAV texture2D;
			Tex2DArrayUAV texture2DArray;
			Tex3DUAV texture3D;
		};
	};

	class UnorderedAccessView : public ResourceView
	{
		UnorderedAccessViewDesc desc;

	protected:
		explicit UnorderedAccessView(const UnorderedAccessViewDesc& desc) : desc(desc)
		{
		}

	public:
		const UnorderedAccessViewDesc& GetDesc() const { return desc; }
	};

	enum class Filter
	{
		MinMagMipPoint = 0,

		MinMagPointMipLinear = 1,

		MinPointMagLinearMipPoint = 4,

		MinPointMagMipLinear = 5,

		MinLinearMagMipPoint = 16,

		MinLinearMagPointMipLinear = 17,

		MinMagLinearMipPoint = 20,

		MinMagMipLinear = 21,

		Anisotropic = 85,

		ComparisonMinMagMipPoint = 128,

		ComparisonMinMagPointMipLinear = 129,

		ComparisonMinPointMagLinearMipPoint = 132,

		ComparisonMinPointMagMipLinear = 133,

		ComparisonMinLinearMagMipPoint = 144,

		ComparisonMinLinearMagPointMipLinear = 145,

		ComparisonMinMagLinearMipPoint = 148,

		ComparisonMinMagMipLinear = 149,

		ComparisonAnisotropic = 213,

		MinimumMinMagMipPoint = 256,

		MinimumMinMagPointMipLinear = 257,

		MinimumMinPointMagLinearMipPoint = 260,

		MinimumMinPointMagMipLinear = 261,

		MinimumMinLinearMagMipPoint = 272,

		MinimumMinLinearMagPointMipLinear = 273,

		MinimumMinMagLinearMipPoint = 276,

		MinimumMinMagMipLinear = 277,

		MinimumAnisotropic = 341,

		MaximumMinMagMipPoint = 384,

		MaximumMinMagPointMipLinear = 385,

		MaximumMinPointMagLinearMipPoint = 388,

		MaximumMinPointMagMipLinear = 389,

		MaximumMinLinearMagMipPoint = 400,

		MaximumMinLinearMagPointMipLinear = 401,

		MaximumMinMagLinearMipPoint = 404,

		MaximumMinMagMipLinear = 405,

		MaximumAnisotropic = 469
	};

	enum class TextureAddressMode
	{
		Wrap = 1,

		Mirror = 2,

		Clamp = 3,

		Border = 4,

		MirrorOnce = 5
	};

	enum class ComparisonFunc
	{
		Never = 1,

		Less = 2,

		Equal = 3,

		LessEqual = 4,

		Greater = 5,

		NotEqual = 6,

		GreaterEqual = 7,

		Always = 8
	};

	struct SamplerDesc
	{
		Filter filter;
		TextureAddressMode addressU;
		TextureAddressMode addressV;
		TextureAddressMode addressW;
		float mipLODBias;
		uint32_t maxAnisotropy;
		ComparisonFunc comparisonFunc;
		Color borderColor;
		float minLOD;
		float maxLOD;
	};

	class SamplerState : public PrismObject
	{
		SamplerDesc desc;

	protected:
		explicit SamplerState(const SamplerDesc& desc) : desc(desc)
		{
		}

	public:
		const SamplerDesc& GetDesc() const { return desc; }
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

	class GraphicsDevice : public PrismObject
	{
	public:
		static PrismObj<GraphicsDevice> Create();
		virtual CommandList* GetImmediateCommandList() = 0;
		virtual PrismObj<Buffer> CreateBuffer(const BufferDesc& desc) = 0;
		virtual PrismObj<Texture1D> CreateTexture1D(const Texture1DDesc& desc) = 0;
		virtual PrismObj<Texture2D> CreateTexture2D(const Texture2DDesc& desc) = 0;
		virtual PrismObj<Texture3D> CreateTexture3D(const Texture3DDesc& desc) = 0;
		virtual PrismObj<RenderTargetView> CreateRenderTargetView(Resource* resource, const RenderTargetViewDesc& desc) = 0;
		virtual PrismObj<ShaderResourceView> CreateShaderResourceView(Resource* resource, const ShaderResourceViewDesc& desc) = 0;
		virtual PrismObj<DepthStencilView> CreateDepthStencilView(Resource* resource, const DepthStencilViewDesc& desc) = 0;
		virtual PrismObj<UnorderedAccessView> CreateUnorderedAccessView(Resource* resource, const UnorderedAccessViewDesc& desc) = 0;
		virtual PrismObj<SamplerState> CreateSamplerState(const SamplerDesc& desc) = 0;
		virtual PrismObj<CommandList> CreateCommandList() = 0;
		virtual PrismObj<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
		virtual PrismObj<GraphicsPipelineState> CreateGraphicsPipelineState(GraphicsPipeline* pipeline, const GraphicsPipelineStateDesc& desc) = 0;
		virtual PrismObj<ComputePipeline> CreateComputePipeline(const ComputePipelineDesc& desc) = 0;
		virtual PrismObj<ComputePipelineState> CreateComputePipelineState(ComputePipeline* pipeline, const ComputePipelineStateDesc& desc) = 0;
	};

HEXA_PRISM_NAMESPACE_END

#pragma once
#include "common.hpp"
#include "prism_base.hpp"
#include "prism_common.hpp"

HEXA_PRISM_NAMESPACE_BEGIN
	class SamplerState;
	class ShaderResourceView;
	class RenderTargetView;
	class UnorderedAccessView;

	struct Viewport
	{
		float X = 0, Y = 0;
		float Width = 0, Height = 0;
		float MinDepth = 0, MaxDepth = 1.0f;

		constexpr Viewport() = default;

		constexpr Viewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
			: X(x), Y(y), Width(width), Height(height), MinDepth(minDepth), MaxDepth(maxDepth)
		{
		}

		constexpr Viewport(float width, float height)
			: X(0.0f), Y(0.0f), Width(width), Height(height), MinDepth(0.0f), MaxDepth(1.0f)
		{
		}

		constexpr Viewport(int width, int height)
			: X(0.0f), Y(0.0f), Width(static_cast<float>(width)), Height(static_cast<float>(height)), MinDepth(0.0f), MaxDepth(1.0f)
		{
		}
	};

	struct Color
	{
		float r, g, b, a;
	};

	class Resource : public PrismObject
	{
	};

	struct SubresourceData
	{
		const void* data;
		uint32_t rowPitch;
		uint32_t slicePitch;
	};

	struct SampleDesc
	{
		uint32_t count;
		uint32_t quality;
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

	class TextShaderSource : public ShaderSource
	{
		std::string identifier;
		std::string text;
	public:
		TextShaderSource(const std::string& identifier, const std::string& text) : identifier(identifier),		 text(text)
		{
		}

		const char* GetIdentifier() override
		{
			return identifier.c_str();
		}

		void GetData(uint8_t*& data, size_t& dataLength) override
		{
			data = reinterpret_cast<uint8_t*>(text.data());
			dataLength = text.size();
		}
	};

	class Blob : public PrismObject
	{
		uint8_t* data;
		size_t length;
		bool owns;

	public:
		Blob() : data(nullptr), length(0), owns(false)
		{
		}

		Blob(uint8_t* bytecode, size_t length, bool owns, bool copy = false) : data(bytecode), length(length), owns(owns)
		{
			if (copy && length > 0)
			{
				data = static_cast<uint8_t*>(PrismAlloc(length));
				PrismMemoryCopy(data, bytecode, length);
				this->owns = true;
			}
		}

		~Blob() override
		{
			if (owns)
			{
				PrismFree(data);
			}
		}


		uint8_t* GetData() const { return data; }
		size_t GetLength() const { return length; }
	};

	class Pipeline : public PrismObject
	{
	};

	struct BindingValuePair
	{
		const char* name;
		ShaderStage stage;
		ShaderParameterType type;
		void* value;
	};

	class ResourceBindingList
	{
	public:
		using iterator = BindingValuePair*;
		using iterator_pair = std::pair<iterator, iterator>;

		virtual ~ResourceBindingList() = default;

		virtual Pipeline* GetPipeline() const = 0;

		virtual void SetCBV(const char* name, Buffer* buffer) = 0;
		virtual void SetSampler(const char* name, SamplerState* sampler) = 0;
		virtual void SetSRV(const char* name, ShaderResourceView* view) = 0;
		virtual void SetUAV(const char* name, UnorderedAccessView* view, uint32_t initialCount = static_cast<uint32_t>(-1)) = 0;

		virtual void SetCBV(const char* name, ShaderStage stage, Buffer* buffer) = 0;
		virtual void SetSampler(const char* name, ShaderStage stage, SamplerState* sampler) = 0;
		virtual void SetSRV(const char* name, ShaderStage stage, ShaderResourceView* view) = 0;
		virtual void SetUAV(const char* name, ShaderStage stage, UnorderedAccessView* view, uint32_t initialCount = static_cast<uint32_t>(-1)) = 0;

		virtual iterator_pair GetSRVs() = 0;
		virtual iterator_pair GetCBVs() = 0;
		virtual iterator_pair GetUAVs() = 0;
		virtual iterator_pair GetSamplers() = 0;
	};

	class PipelineState : public PrismObject
	{
	public:
		virtual ResourceBindingList& GetBindings() = 0;
	};

	struct GraphicsPipelineDesc
	{
		PrismObj<ShaderSource> vertexShader;
		const char* vertexEntryPoint;
		PrismObj<ShaderSource> hullShader;
		const char* hullEntryPoint;
		PrismObj<ShaderSource> domainShader;
		const char* domainEntryPoint;
		PrismObj<ShaderSource> geometryShader;
		const char* geometryEntryPoint;
		PrismObj<ShaderSource> pixelShader;
		const char* pixelEntryPoint;
	};

	class GraphicsPipeline : public Pipeline
	{
	protected:
		GraphicsPipelineDesc desc;
	public:
		GraphicsPipeline(const GraphicsPipelineDesc& desc) : desc(desc) {}

		const GraphicsPipelineDesc& GetDesc() const { return desc; }
	};

	struct RenderTargetBlendDescription
	{
		bool isBlendEnabled = false;
		bool isLogicOpEnabled = false;

		Blend sourceBlend = Blend::One;
		Blend destinationBlend = Blend::Zero;
		BlendOperation blendOp = BlendOperation::Add;
		Blend sourceBlendAlpha = Blend::One;
		Blend destinationBlendAlpha = Blend::Zero;
		BlendOperation blendOpAlpha = BlendOperation::Add;

		LogicOperation logicOp = LogicOperation::Clear;
		ColorWriteEnable renderTargetWriteMask = ColorWriteEnable::All;

		constexpr RenderTargetBlendDescription() = default;
	};

	struct BlendDescription
	{
		static constexpr size_t SimultaneousRenderTargetCount = (8);

		bool alphaToCoverageEnable = false;
		bool independentBlendEnable = false;
		std::array<RenderTargetBlendDescription, SimultaneousRenderTargetCount> renderTargets;

	private:
		constexpr bool IsBlendEnabled(const RenderTargetBlendDescription& renderTarget)
		{
			return renderTarget.blendOpAlpha != BlendOperation::Add
				|| renderTarget.sourceBlendAlpha != Blend::One
				|| renderTarget.destinationBlendAlpha != Blend::Zero
				|| renderTarget.blendOp != BlendOperation::Add
				|| renderTarget.sourceBlend != Blend::One
				|| renderTarget.destinationBlend != Blend::Zero;
		}

	public:
		constexpr BlendDescription() = default;

		constexpr BlendDescription(Blend sourceBlend, Blend destinationBlend)
			: BlendDescription(sourceBlend, destinationBlend, sourceBlend, destinationBlend)
		{
		}

		constexpr BlendDescription(Blend sourceBlend, Blend destinationBlend, Blend srcBlendAlpha, Blend destBlendAlpha)
			: BlendDescription()
		{
			alphaToCoverageEnable = false;
			independentBlendEnable = false;

			for (int i = 0; i < SimultaneousRenderTargetCount; i++)
			{
				renderTargets[i].sourceBlend = sourceBlend;
				renderTargets[i].destinationBlend = destinationBlend;
				renderTargets[i].blendOp = BlendOperation::Add;
				renderTargets[i].sourceBlendAlpha = srcBlendAlpha;
				renderTargets[i].destinationBlendAlpha = destBlendAlpha;
				renderTargets[i].blendOpAlpha = BlendOperation::Add;
				renderTargets[i].renderTargetWriteMask = ColorWriteEnable::All;
				renderTargets[i].isBlendEnabled = IsBlendEnabled(renderTargets[i]);
				renderTargets[i].isLogicOpEnabled = false;
			}
		}

		constexpr BlendDescription(Blend sourceBlend, Blend destinationBlend, Blend srcBlendAlpha, Blend destBlendAlpha, BlendOperation blendOperation, BlendOperation blendOperationAlpha)
			: BlendDescription()
		{
			alphaToCoverageEnable = false;
			independentBlendEnable = false;

			for (size_t i = 0; i < SimultaneousRenderTargetCount; i++)
			{
				renderTargets[i].sourceBlend = sourceBlend;
				renderTargets[i].destinationBlend = destinationBlend;
				renderTargets[i].blendOp = blendOperation;
				renderTargets[i].sourceBlendAlpha = srcBlendAlpha;
				renderTargets[i].destinationBlendAlpha = destBlendAlpha;
				renderTargets[i].blendOpAlpha = blendOperationAlpha;
				renderTargets[i].renderTargetWriteMask = ColorWriteEnable::All;
				renderTargets[i].isBlendEnabled = IsBlendEnabled(renderTargets[i]);
				renderTargets[i].isLogicOpEnabled = false;
			}
		}

		constexpr BlendDescription(Blend sourceBlend, Blend destinationBlend, Blend srcBlendAlpha, Blend destBlendAlpha, BlendOperation blendOperation, BlendOperation blendOperationAlpha, LogicOperation logicOperation)
			: BlendDescription()
		{
			alphaToCoverageEnable = false;
			independentBlendEnable = false;

			for (size_t i = 0; i < SimultaneousRenderTargetCount; i++)
			{
				renderTargets[i].sourceBlend = sourceBlend;
				renderTargets[i].destinationBlend = destinationBlend;
				renderTargets[i].blendOp = blendOperation;
				renderTargets[i].sourceBlendAlpha = srcBlendAlpha;
				renderTargets[i].destinationBlendAlpha = destBlendAlpha;
				renderTargets[i].blendOpAlpha = blendOperationAlpha;
				renderTargets[i].logicOp = logicOperation;
				renderTargets[i].isLogicOpEnabled = true;
				renderTargets[i].renderTargetWriteMask = ColorWriteEnable::All;
				renderTargets[i].isBlendEnabled = IsBlendEnabled(renderTargets[i]);
			}
		}
	};

	namespace BlendDescriptions
	{
		static constexpr BlendDescription Opaque = BlendDescription(Blend::One, Blend::Zero);
		static constexpr BlendDescription AlphaBlend = BlendDescription(Blend::One, Blend::InverseSourceAlpha);
		static constexpr BlendDescription Additive = BlendDescription(Blend::SourceAlpha, Blend::One);
		static constexpr BlendDescription NonPremultiplied = BlendDescription(Blend::SourceAlpha, Blend::InverseSourceAlpha);
	}

	struct RasterizerDescription
	{
		static constexpr int DefaultDepthBias = 0;
		static constexpr float DefaultDepthBiasClamp = 0.0f;
		static constexpr float DefaultSlopeScaledDepthBias = 0.0f;

		FillMode fillMode = FillMode::Solid;
		CullMode cullMode = CullMode::Back;
		bool frontCounterClockwise = false;
		int depthBias = DefaultDepthBias;
		float depthBiasClamp = DefaultDepthBiasClamp;
		float slopeScaledDepthBias = DefaultSlopeScaledDepthBias;
		bool depthClipEnable = true;
		bool scissorEnable = false;
		bool multisampleEnable = true;
		bool antialiasedLineEnable = false;
		uint32_t forcedSampleCount = 0;
		ConservativeRasterizationMode conservativeRaster = ConservativeRasterizationMode::Off;

		constexpr RasterizerDescription() = default;

		constexpr RasterizerDescription(enum CullMode cullMode, enum FillMode fillMode)
			: fillMode(fillMode), cullMode(cullMode)
		{
		}

		constexpr RasterizerDescription(
			enum CullMode cullMode,
			enum FillMode fillMode,
			bool frontCounterClockwise,
			int depthBias,
			float depthBiasClamp,
			float slopeScaledDepthBias,
			bool depthClipEnable,
			bool scissorEnable,
			bool multisampleEnable,
			bool antialiasedLineEnable)
			: fillMode(fillMode),
			  cullMode(cullMode),
			  frontCounterClockwise(frontCounterClockwise),
			  depthBias(depthBias),
			  depthBiasClamp(depthBiasClamp),
			  slopeScaledDepthBias(slopeScaledDepthBias),
			  depthClipEnable(depthClipEnable),
			  scissorEnable(scissorEnable),
			  multisampleEnable(multisampleEnable),
			  antialiasedLineEnable(antialiasedLineEnable)
		{
		}

		constexpr RasterizerDescription(
			enum CullMode cullMode,
			enum FillMode fillMode,
			bool frontCounterClockwise,
			int depthBias,
			float depthBiasClamp,
			float slopeScaledDepthBias,
			bool depthClipEnable,
			bool scissorEnable,
			bool multisampleEnable,
			bool antialiasedLineEnable,
			uint32_t forcedSampleCount,
			ConservativeRasterizationMode conservativeRasterization)
			: fillMode(fillMode),
			  cullMode(cullMode),
			  frontCounterClockwise(frontCounterClockwise),
			  depthBias(depthBias),
			  depthBiasClamp(depthBiasClamp),
			  slopeScaledDepthBias(slopeScaledDepthBias),
			  depthClipEnable(depthClipEnable),
			  scissorEnable(scissorEnable),
			  multisampleEnable(multisampleEnable),
			  antialiasedLineEnable(antialiasedLineEnable),
			  forcedSampleCount(forcedSampleCount),
			  conservativeRaster(conservativeRasterization)
		{
		}
	};

	namespace RasterizerDescriptions
	{
		static constexpr RasterizerDescription CullNone = RasterizerDescription(CullMode::None, FillMode::Solid);
		static constexpr RasterizerDescription CullFront = RasterizerDescription(CullMode::Front, FillMode::Solid);
		static constexpr RasterizerDescription CullBack = RasterizerDescription(CullMode::Back, FillMode::Solid);
		static constexpr RasterizerDescription CullBackScissors = RasterizerDescription(CullMode::Back, FillMode::Solid, false, 0, 0.0f, 0.0f, true, true, true, false);
		static constexpr RasterizerDescription Wireframe = RasterizerDescription(CullMode::None, FillMode::Wireframe);
		static constexpr RasterizerDescription CullNoneDepthBias = RasterizerDescription(CullMode::None, FillMode::Solid, false, -1, 0.0f, 1.0f, true, false, false, false);
		static constexpr RasterizerDescription CullFrontDepthBias = RasterizerDescription(CullMode::Front, FillMode::Solid, false, -1, 0.0f, 1.0f, true, false, false, false);
		static constexpr RasterizerDescription CullBackDepthBias = RasterizerDescription(CullMode::Back, FillMode::Solid, false, -1, 0.0f, 1.0f, true, false, false, false);
	}

	struct DepthStencilOperationDescription
	{
		StencilOperation stencilFailOp = StencilOperation::Keep;
		StencilOperation stencilDepthFailOp = StencilOperation::Keep;
		StencilOperation stencilPassOp = StencilOperation::Keep;
		ComparisonFunc stencilFunc = ComparisonFunc::Always;

		constexpr DepthStencilOperationDescription() = default;

		constexpr DepthStencilOperationDescription(
			StencilOperation stencilFailOp,
			StencilOperation stencilDepthFailOp,
			StencilOperation stencilPassOp,
			ComparisonFunc stencilFunc)
			: stencilFailOp(stencilFailOp),
			  stencilDepthFailOp(stencilDepthFailOp),
			  stencilPassOp(stencilPassOp),
			  stencilFunc(stencilFunc)
		{
		}
	};	

	namespace DepthStencilOperationDescriptions
	{
		static constexpr DepthStencilOperationDescription Default = DepthStencilOperationDescription(
			StencilOperation::Keep,
			StencilOperation::Keep,
			StencilOperation::Keep,
			ComparisonFunc::Always);

		static constexpr DepthStencilOperationDescription DefaultFront = DepthStencilOperationDescription(
			StencilOperation::Keep,
			StencilOperation::Increment,
			StencilOperation::Keep,
			ComparisonFunc::Always);

		static constexpr DepthStencilOperationDescription DefaultBack = DepthStencilOperationDescription(
			StencilOperation::Keep,
			StencilOperation::Decrement,
			StencilOperation::Keep,
			ComparisonFunc::Always);
	}

	struct DepthStencilDescription
	{
		static constexpr uint8_t DefaultStencilReadMask = 255;
		static constexpr uint8_t DefaultStencilWriteMask = 255;

		bool depthEnable = true;
		DepthWriteMask depthWriteMask = DepthWriteMask::All;
		ComparisonFunc depthFunc = ComparisonFunc::LessEqual;
		bool stencilEnable = false;
		uint8_t stencilReadMask = DefaultStencilReadMask;
		uint8_t stencilWriteMask = DefaultStencilWriteMask;
		DepthStencilOperationDescription frontFace;
		DepthStencilOperationDescription backFace;

		constexpr DepthStencilDescription() = default;

		constexpr DepthStencilDescription(bool depthEnable, DepthWriteMask depthWriteMask, ComparisonFunc depthFunc = ComparisonFunc::LessEqual)
			: depthEnable(depthEnable),
			  depthWriteMask(depthWriteMask),
			  depthFunc(depthFunc),
			  stencilEnable(false),
			  stencilReadMask(DefaultStencilReadMask),
			  stencilWriteMask(DefaultStencilWriteMask),
			  frontFace(DepthStencilOperationDescriptions::Default),
			  backFace(DepthStencilOperationDescriptions::Default)
		{
		}

		constexpr DepthStencilDescription(bool depthEnable, bool stencilEnable, DepthWriteMask depthWriteMask, ComparisonFunc depthFunc = ComparisonFunc::LessEqual)
			: depthEnable(depthEnable),
			  depthWriteMask(depthWriteMask),
			  depthFunc(depthFunc),
			  stencilEnable(stencilEnable),
			  stencilReadMask(DefaultStencilReadMask),
			  stencilWriteMask(DefaultStencilWriteMask),
			  frontFace(DepthStencilOperationDescriptions::DefaultFront),
			  backFace(DepthStencilOperationDescriptions::DefaultBack)
		{
		}

		constexpr DepthStencilDescription(
			bool depthEnable,
			bool depthWriteEnable,
			ComparisonFunc depthFunc,
			bool stencilEnable,
			uint8_t stencilReadMask,
			uint8_t stencilWriteMask,
			StencilOperation frontStencilFailOp,
			StencilOperation frontStencilDepthFailOp,
			StencilOperation frontStencilPassOp,
			ComparisonFunc frontStencilFunc,
			StencilOperation backStencilFailOp,
			StencilOperation backStencilDepthFailOp,
			StencilOperation backStencilPassOp,
			ComparisonFunc backStencilFunc)
			: depthEnable(depthEnable),
			  depthWriteMask(depthWriteEnable ? DepthWriteMask::All : DepthWriteMask::Zero),
			  depthFunc(depthFunc),
			  stencilEnable(stencilEnable),
			  stencilReadMask(stencilReadMask),
			  stencilWriteMask(stencilWriteMask),
			  frontFace(frontStencilFailOp, frontStencilDepthFailOp, frontStencilPassOp, frontStencilFunc),
			  backFace(backStencilFailOp, backStencilDepthFailOp, backStencilPassOp, backStencilFunc)
		{
		}
	};

	namespace DepthStencilDescriptions
	{
		static constexpr DepthStencilDescription None = DepthStencilDescription(false, DepthWriteMask::Zero);
		static constexpr DepthStencilDescription Always = DepthStencilDescription(true, DepthWriteMask::All, ComparisonFunc::Always);
		static constexpr DepthStencilDescription Default = DepthStencilDescription(true, DepthWriteMask::All);
		static constexpr DepthStencilDescription DefaultLess = DepthStencilDescription(true, DepthWriteMask::All, ComparisonFunc::Less);
		static constexpr DepthStencilDescription DefaultStencil = DepthStencilDescription(true, true, DepthWriteMask::All);
		static constexpr DepthStencilDescription DepthRead = DepthStencilDescription(true, DepthWriteMask::Zero);
		static constexpr DepthStencilDescription DepthReadEquals = DepthStencilDescription(true, DepthWriteMask::Zero, ComparisonFunc::Equal);
		static constexpr DepthStencilDescription DepthReverseZ = DepthStencilDescription(true, DepthWriteMask::All, ComparisonFunc::GreaterEqual);
		static constexpr DepthStencilDescription DepthReadReverseZ = DepthStencilDescription(true, DepthWriteMask::Zero, ComparisonFunc::GreaterEqual);
	}

	struct InputElementDescription
	{
		const char* semanticName;
		uint32_t semanticIndex;
		Format format;
		uint32_t slot;
		uint32_t alignedByteOffset;
		InputClassification classification;
		uint32_t instanceDataStepRate;

		static constexpr uint32_t AppendAligned = static_cast<uint32_t>(-1);
	};

	struct GraphicsPipelineStateDesc
	{
		RasterizerDescription rasterizer = RasterizerDescriptions::CullBack;
		DepthStencilDescription depthStencil = DepthStencilDescriptions::Default;
		BlendDescription blend = BlendDescriptions::Opaque;
		PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
		Color blendFactor;
		uint32_t sampleMask = 0xFFFFFFFF;
		uint32_t stencilRef = 0;
		const InputElementDescription* inputElements = nullptr;
		size_t numInputElements = 0;
		PipelineStateFlags flags = PipelineStateFlags::None;
	};

	class GraphicsPipelineState : public PipelineState
	{
	protected:
		PrismObj<GraphicsPipeline> pipeline;
		GraphicsPipelineStateDesc desc;

	public:
		GraphicsPipelineState(PrismObj<GraphicsPipeline> pipeline, const GraphicsPipelineStateDesc& desc)
			: pipeline(pipeline), desc(desc)
		{
		}

		const GraphicsPipelineStateDesc& GetDesc() const { return desc; }
		const PrismObj<GraphicsPipeline>& GetPipeline() const { return pipeline; }
	};

	struct ComputePipelineDesc
	{
		PrismObj<ShaderSource> computeShader;
		const char* computeEntryPoint;
	};

	class ComputePipeline : public Pipeline
	{
	protected:
		ComputePipelineDesc desc;
	public:
		ComputePipeline(const ComputePipelineDesc& desc) : desc(desc) {}
		const ComputePipelineDesc& GetDesc() const { return desc; }
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

	struct SwapChainDesc
	{
		uint32_t width;
		uint32_t height;
		Format format;
		bool stereo;
		SampleDesc sampleDesc;
		Usage bufferUsage;
		uint32_t bufferCount;
		Scaling scaling;
		SwapEffect swapEffect;
		AlphaMode alphaMode;
		SwapChainFlags flags;
	};

	struct Rational
	{
		uint32_t numerator;
		uint32_t denominator;
	};

	struct SwapChainFullscreenDesc 
	{
		Rational refreshRate;
		ScanlineOrder scanlineOrdering;
		Scaling scaling;
		bool windowed;
	};

	class SwapChain : public PrismObject
	{
	protected:
		SwapChainDesc desc;
		SwapChainFullscreenDesc fullscreenDesc;

	public:
		SwapChain(const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc) 
			: desc(desc), fullscreenDesc(fullscreenDesc)
		{
		}

		const SwapChainDesc& GetDesc() const { return desc; }
		const SwapChainFullscreenDesc& GetFullscreenDesc() const { return fullscreenDesc; }

		virtual void ResizeBuffers(uint32_t bufferCount, uint32_t width, uint32_t height, Format newFormat, SwapChainFlags swapChainFlags) = 0;

		virtual PrismObj<Texture2D> GetBuffer(size_t index) = 0;
		virtual void Present(uint32_t interval, PresentFlags flags) = 0;
	};

	class CommandList : public PrismObject
	{
	public:
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
		virtual PrismObj<Buffer> CreateBuffer(const BufferDesc& desc, const SubresourceData* initialData = nullptr) = 0;
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
		virtual PrismObj<SwapChain> CreateSwapChain(void* windowHandle, const SwapChainDesc& desc, const SwapChainFullscreenDesc& fullscreenDesc) = 0;
		virtual PrismObj<SwapChain> CreateSwapChain(void* windowHandle) = 0;
	};

HEXA_PRISM_NAMESPACE_END

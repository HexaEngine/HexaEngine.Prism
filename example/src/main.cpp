#include "main.hpp"
#include <prism.hpp>
#include <prism_texture_loader.hpp>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>

using namespace HEXA_PRISM_NAMESPACE;

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Window* window = SDL_CreateWindow("Test", static_cast<int>(1280 * scale), static_cast<int>(720 * scale), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_VULKAN);

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    CommandQueueDesc queueDesc = { CommandQueueType::Graphics, 0, 1.0f };

    DeviceDesc desc{};
    desc.type = BackendType::Vulkan;
    desc.flags = DeviceFlags::Win32 | DeviceFlags::Debug;
    desc.queues = &queueDesc;
    desc.queuesCount = 1;
    desc.debugCallback = [](DebugMessageSeverity severity, const char* message, void* userData)
    {
        const char* prefix = severity == DebugMessageSeverity::Error ? "[Error] "
            : severity == DebugMessageSeverity::Warning ? "[Warning] "
            : severity == DebugMessageSeverity::Info ? "[Info] "
            : "[Verbose] ";
        std::cout << prefix << message << std::endl;
    };
    auto device = PrismDevice::Create(desc);
    auto queue = device->GetCommandQueue(0);
    auto allocator = device->CreateCommandAllocator(CommandListType::Direct);
    
    CommandListDesc commandListDesc = { CommandListType::Direct, allocator.Get() };
    auto ctx = device->CreateCommandList(commandListDesc);
	
    auto swapChain = device->CreateSwapChain(window);
    if (!swapChain)
    {
        std::cerr << "Failed to create swap chain" << std::endl;
        return 1;
    }
    // One RTV per swapchain buffer, created once and reused every frame.
    std::vector<PrismObj<RenderTargetView>> backBufferRtvs;
    for (uint32_t i = 0; i < swapChain->GetDesc().bufferCount; ++i)
    {
        auto tex = swapChain->GetBuffer(i);
        RenderTargetViewDesc rtvDesc = {};
        rtvDesc.dimension = RenderTargetViewDimension::Texture2D;
        backBufferRtvs.push_back(device->CreateRenderTargetView(tex, rtvDesc));
    }

    auto vertexShader = MakePrismObj<TextShaderSource>("VertexShader", R"(struct VSInput
    {
        float3 pos : POSITION;
        float4 color : COLOR;
        float2 uv : TEXCOORD;
    };

    struct PSInput
    {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
        float2 uv : TEXCOORD;
    };

	cbuffer constantBuffer
	{
		float4x4 transform;
	};

    PSInput main(VSInput input)
    {
        PSInput output;
        output.pos = mul(float4(input.pos, 1.0), transform);
        output.color = input.color;
        output.uv = input.uv;
        return output;
    }
    )");
	auto pixelShader = MakePrismObj<TextShaderSource>("PixelShader", R"(struct PSInput
    {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
        float2 uv : TEXCOORD;
    };

    Texture2D tex;
    SamplerState samp;

    float4 main(PSInput input) : SV_TARGET
    {
        return input.color * tex.Sample(samp, input.uv);
    }
    )");

	GraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.pixelShader = pixelShader;

	auto pipeline = device->CreateGraphicsPipeline(pipelineDesc);

	GraphicsPipelineStateDesc psoDesc = {};
    psoDesc.numRenderTargets = 1;
    psoDesc.renderTargetFormats[0] = swapChain->GetDesc().format;
    auto pso = device->CreateGraphicsPipelineState(pipeline, psoDesc);

	BufferDesc vertexBufferDesc = {};
	vertexBufferDesc.type = BufferType::VertexBuffer;
	vertexBufferDesc.widthInBytes = sizeof(float) * 9 * 3;
	vertexBufferDesc.cpuAccessFlags = CpuAccessFlags::None;
	vertexBufferDesc.gpuAccessFlags = GpuAccessFlags::Immutable;

	SubresourceData initialData = {};
    float vertexData[] = {
        // Position           // Color              // UV
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
	};

	initialData.data = vertexData;
	auto vertexBuffer = device->CreateBuffer(vertexBufferDesc, &initialData);

	// Drop your own DDS/TGA/HDR/PNG/JPG file at this path (relative to the working directory).
	TextureLoader textureLoader(device);
	auto texture = textureLoader.LoadTexture2D("C:\\Users\\junam\\Desktop\\itjustworks.png");

	ShaderResourceViewDesc srvDesc = {};
	srvDesc.dimension = ShaderResourceViewDimension::Texture2D;
	srvDesc.texture2D.mostDetailedMip = 0;
	srvDesc.texture2D.mipLevels = texture->GetDesc().mipLevels;
	auto textureSrv = device->CreateShaderResourceView(texture, srvDesc);

	SamplerDesc samplerDesc = {};
	samplerDesc.filter = Filter::MinMagMipLinear;
	samplerDesc.addressU = TextureAddressMode::Wrap;
	samplerDesc.addressV = TextureAddressMode::Wrap;
	samplerDesc.addressW = TextureAddressMode::Wrap;
	samplerDesc.maxAnisotropy = 1;
	samplerDesc.minLOD = 0.0f;
	samplerDesc.maxLOD = 1000.0f;
	auto sampler = device->CreateSamplerState(samplerDesc);

	pso->GetBindings().SetSRV("tex", textureSrv);
	pso->GetBindings().SetSampler("samp", sampler);

    BufferDesc constantBufferDesc = {};
	constantBufferDesc.type = BufferType::ConstantBuffer;
	constantBufferDesc.widthInBytes = sizeof(float) * 16;
	constantBufferDesc.cpuAccessFlags = CpuAccessFlags::Write;
	constantBufferDesc.gpuAccessFlags = GpuAccessFlags::None;
	auto constantBuffer = device->CreateBuffer(constantBufferDesc);
    pso->GetBindings().SetCBV("constantBuffer", constantBuffer);

	bool running = true;
    while (running)
    {
        SDL_PumpEvents();
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
		}

        float transformData[] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        RenderTargetView* rtv = backBufferRtvs[swapChain->GetCurrentBackBufferIndex()].Get();

        ctx->Begin();
        ctx->WriteArray(constantBuffer, transformData, 16);

        ctx->ClearRenderTargetView(rtv, { 0.3f,0.3f,0.3f,1 });
		ctx->SetVertexBuffer(0, vertexBuffer, sizeof(float) * 9, 0);
        ctx->SetRenderTarget(rtv, nullptr);
		ctx->SetViewport({width, height});
		ctx->SetGraphicsPipelineState(pso);
		ctx->DrawInstanced(3, 1, 0, 0);

        ctx->End();

        CommandList* lists[] = { ctx.Get() };
        queue->Submit(lists, 1);

		swapChain->Present(0, PresentFlags::None);
    }
    
    return 0;
}
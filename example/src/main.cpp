#include "main.hpp"
#include <prism.hpp>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>

using namespace HEXA_PRISM_NAMESPACE;

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Window* window = SDL_CreateWindow("Test", static_cast<int>(1280 * scale), static_cast<int>(720 * scale), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    PrismObj<GraphicsDevice> device = GraphicsDevice::Create();
	
    auto ctx = device->GetImmediateCommandList();
    auto swapChain = device->CreateSwapChain(window);
    auto tex = swapChain->GetBuffer(0);
    RenderTargetViewDesc rtvDesc = {};
    rtvDesc.dimension = RenderTargetViewDimension::Texture2D;
    auto rtv = device->CreateRenderTargetView(tex, rtvDesc);

    auto vertexShader = MakePrismObj<TextShaderSource>("VertexShader", R"(struct VSInput
    {
        float3 pos : POSITION;
        float4 color : COLOR;
    };

    struct PSInput
    {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
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
        return output;
    }
    )");
	auto pixelShader = MakePrismObj<TextShaderSource>("PixelShader", R"(struct PSInput
    {
        float4 pos : SV_POSITION;
        float4 color : COLOR;
    };

    float4 main(PSInput input) : SV_TARGET
    {
        return input.color;
    }
    )");

	GraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.pixelShader = pixelShader;

	auto pipeline = device->CreateGraphicsPipeline(pipelineDesc);

	GraphicsPipelineStateDesc psoDesc = {};
    auto pso = device->CreateGraphicsPipelineState(pipeline, psoDesc);

	BufferDesc vertexBufferDesc = {};
	vertexBufferDesc.type = BufferType::VertexBuffer;
	vertexBufferDesc.widthInBytes = sizeof(float) * 7 * 3;
	vertexBufferDesc.cpuAccessFlags = CpuAccessFlags::None;
	vertexBufferDesc.gpuAccessFlags = GpuAccessFlags::Immutable;

	SubresourceData initialData = {};
    float vertexData[] = {
        // Position         // Color
         0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	};

	initialData.data = vertexData;
	auto vertexBuffer = device->CreateBuffer(vertexBufferDesc, &initialData);

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

        ctx->WriteArray(constantBuffer, transformData, 16);

        ctx->ClearRenderTargetView(rtv, { 0.3f,0.3f,0.3f,1 });
		ctx->SetVertexBuffer(0, vertexBuffer, sizeof(float) * 7, 0);
        ctx->SetRenderTarget(rtv, nullptr);
		ctx->SetViewport({width, height});
		ctx->SetGraphicsPipelineState(pso);
		ctx->DrawInstanced(3, 1, 0, 0);

		swapChain->Present(0, PresentFlags::None);
    }
    
    return 0;
}
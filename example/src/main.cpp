#include "main.hpp"
#include <graphics.hpp>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>

#if HEXA_PRISM_WINDOWS
#include <Windows.h>

void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

#else
void HideConsole()
{
}

#endif

using namespace HEXA_PRISM_NAMESPACE;

int main()
{
    HideConsole();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Window* window = SDL_CreateWindow("Test", static_cast<int>(1280 * scale), static_cast<int>(720 * scale), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    PrismObj<GraphicsDevice> device = GraphicsDevice::Create();
	
    auto ctx = device->GetImmediateCommandList();
    auto swapChain = device->CreateSwapChain(window);
    auto tex = swapChain->GetBuffer(0);
    RenderTargetViewDesc rtvDesc = {};
    rtvDesc.dimension = RenderTargetViewDimension::Texture2D;
    auto rtv = device->CreateRenderTargetView(tex, rtvDesc);

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

        ctx->ClearRenderTargetView(rtv, {0.3f,0.3f,0.3f,1});
		swapChain->Present(1, PresentFlags::None);
    }
    
    return 0;
}
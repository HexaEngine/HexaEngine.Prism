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
    SDL_Window* window = SDL_CreateWindow("Test", (int)(1280 * scale), (int)(720 * scale), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    PrismObj<GraphicsDevice> device = GraphicsDevice::Create();
    Texture2DDesc desc = {};
	desc.format = Format::RGBA8_UNorm;
	desc.width = 256;
	desc.height = 256;
	desc.arraySize = 1;
	desc.mipLevels = 1;
	desc.cpuAccessFlags = CpuAccessFlags::None;
	desc.gpuAccessFlags = GpuAccessFlags::RW;
	desc.sampleDesc = { 1, 0 };
    auto tex = device->CreateTexture2D(desc);

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


    }
    
    return 0;
}
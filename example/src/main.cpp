#include "main.hpp"
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("Test", 1280, 720, SDL_WINDOW_RESIZABLE);

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
#include "platform.hpp"

#include <SDL2/SDL.h>

namespace
{
	static SDL_Window* s_window = nullptr;
	static SDL_Renderer* s_renderer = nullptr;
}

void Platform::panic(const char* fmt, ...)
{
	Log::error(fmt);
	SDL_ShowSimpleMessageBox(0, "Fatal Error", fmt, s_window);
	exit(EXIT_FAILURE);
}

void Platform::init()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		panic(SDL_GetError());
	}

	const int w = 1280, h = 720;
	s_window = SDL_CreateWindow("App",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!s_window) {
		panic(SDL_GetError());
	}

	s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_ACCELERATED);
	if (!s_renderer){
		panic(SDL_GetError());
	}

	atexit(Platform::deinit);
}

void Platform::deinit()
{
	SDL_DestroyRenderer(s_renderer);
	SDL_DestroyWindow(s_window);
	SDL_Quit();
}

void Platform::run()
{
	SDL_Event e;
	while (true) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				return;
		}
	}
}

void Log::debug(const char* fmt, ...)
{
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, fmt);
}

void Log::info(const char* fmt, ...)
{
	SDL_Log(fmt);
}

void Log::warn(const char* fmt, ...)
{
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, fmt);
}

void Log::error(const char* fmt, ...)
{
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, fmt);
}

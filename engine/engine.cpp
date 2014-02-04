#include "engine.hpp"

#include <SDL2/SDL.h>

namespace
{
	static SDL_Window* s_window = nullptr;
	static SDL_GLContext s_glcontext = nullptr;
	static int s_width = 0;
	static int s_height = 0;
}

void Engine::init()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		panic(SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	std::string err;
	Json settings = Json::parse(readFile("settings.json"), err);
	if (!err.empty())
		panic(err.c_str());

	s_width = settings["screen"]["width"].int_value();
	s_height = settings["screen"]["height"].int_value();
	int fullscreen = settings["screen"]["fullscreen"].bool_value() ? SDL_WINDOW_FULLSCREEN : 0;

	s_window = SDL_CreateWindow("App",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, s_width, s_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | fullscreen);
	if (!s_window) {
		panic(SDL_GetError());
	}

	s_glcontext = SDL_GL_CreateContext(s_window);
	if (!s_glcontext) {
		panic(SDL_GetError());
	}

	int major, minor;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	logInfo("OpenGL Context:  %d.%d", major, minor);

	//atexit(Engine::deinit);
}

void Engine::deinit()
{
	SDL_GL_DeleteContext(s_glcontext);
	SDL_DestroyWindow(s_window);
	SDL_Quit();
}

void Engine::swap()
{
	SDL_GL_SwapWindow(s_window);
}

int Engine::width()
{
	return s_width;
}

int Engine::height()
{
	return s_height;
}

void Engine::grabMouse(bool grab)
{
	SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
}

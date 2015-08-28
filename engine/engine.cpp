#include "engine.hpp"

#include <SDL2/SDL.h>

namespace
{
	static SDL_Window* s_window = nullptr;
	static SDL_GLContext s_glcontext = nullptr;
	static int s_width = 0;
	static int s_height = 0;
	static Uint64 prevTime = 0;
}

float Engine::dt = 0;
Json Engine::settings = Json();

void Engine::init(const string& configPath)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		panic(SDL_GetError());
	}

	logInfo("Logical CPU cores: %d", SDL_GetCPUCount());
	//logInfo("System RAM: %dMB", SDL_GetSystemRAM());
	logInfo("L1 cache line size: %dkB", SDL_GetCPUCacheLineSize());
	logInfo("SSE: %d, SSE2: %d, SSE3: %d, SSE4.1: %d, SSE4.2: %d",
			SDL_HasSSE(), SDL_HasSSE2(), SDL_HasSSE3(), SDL_HasSSE41(), SDL_HasSSE42());

	prevTime = SDL_GetPerformanceCounter();

	std::string err;
	settings = Json::parse(readFile(configPath), err);
	if (!err.empty())
		panic("Error reading config from \"%s\": %s", configPath.c_str(), err.c_str());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	int contextFlags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
	if (settings["renderer"]["gldebug"].bool_value())
		contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contextFlags);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	int msaa = settings["renderer"]["msaa"].int_value();
	if (msaa > 1) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa);
	}

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

	vsync(settings["rendering"]["vsync"].bool_value());

	int major, minor;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	logInfo("OpenGL Context:  %d.%d", major, minor);
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
	Uint64 curTime = SDL_GetPerformanceCounter();
	dt = (curTime - prevTime) / (float)SDL_GetPerformanceFrequency();
	prevTime = curTime;
}

void Engine::vsync(bool enable)
{
	if (SDL_GL_SetSwapInterval(enable ? 1 : 0))
		logError("V-sync %s failed: %s", enable ? "enabling" : "disabling", SDL_GetError());
	else logInfo("V-sync %s", enable ? "enabled" : "disabled");
}

bool Engine::vsync()
{
	return SDL_GL_GetSwapInterval() != 0;
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

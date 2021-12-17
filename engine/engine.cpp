#include "engine.hpp"
#include "utils.hpp"
#include "image.hpp"

#include <SDL.h>
#ifdef _WIN32
#include "glad/glad.h"
#endif

Json Engine::settings = Json();
Engine* Engine::s_singleton = nullptr;

Engine::Engine()
{
	s_singleton = this;
}

Engine::~Engine()
{
	s_singleton = nullptr;
}

void Engine::init(const string& configPath)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		panic("SDL_INIT_VIDEO failed: %s", SDL_GetError());
	}

	logInfo("Logical CPU cores: %d, RAM: %dMB, L1 cache line size: %dkB", SDL_GetCPUCount(), SDL_GetSystemRAM(), SDL_GetCPUCacheLineSize());
	logInfo("%s%s%s%s%s%s", SDL_HasSSE()?"SSE ":"", SDL_HasSSE2()?"SSE2 ":"",
		SDL_HasSSE3()?"SSE3 ":"", SDL_HasSSE41()?"SSE4.1 ":"", SDL_HasSSE42()?"SSE4.2 ":"", SDL_HasAVX()?"AVX ":"");

	m_prevTime = SDL_GetPerformanceCounter();

	std::string err;
	settings = Json::parse(utils::readFile(configPath), err);
	if (!err.empty())
		panic("Error reading config from \"%s\": %s", configPath.c_str(), err.c_str());

	int contextFlags = 0;

	string profile = settings["renderer"]["profile"].string_value();
	if (profile == "es")
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	else if (profile == "compatibility")
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	else {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		contextFlags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
	}

	int versionMajor = 4;
	int versionMinor = 3;
	if (settings["renderer"]["version"].is_array())
	{
		const auto& verArr = settings["renderer"]["version"].array_items();
		versionMajor = verArr[0].int_value();
		versionMinor = verArr[1].int_value();
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, versionMajor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, versionMinor);
	//logDebug("Requesting %s %d.%d", profile.c_str(), versionMajor, versionMinor);

	if (settings["renderer"]["gldebug"].bool_value())
		contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
	if (contextFlags != 0)
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

	m_width = settings["screen"]["width"].int_value();
	m_height = settings["screen"]["height"].int_value();
	int fullscreen = settings["screen"]["fullscreen"].bool_value() ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

	window = SDL_CreateWindow("Weep",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | fullscreen);
	if (!window) {
		panic("Window creation failed: %s", SDL_GetError());
	}

	glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		panic("Context creation failed: %s", SDL_GetError());
	}

	moduleInit();

	vsync(settings["renderer"]["vsync"].bool_value());

	int major, minor;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	logInfo("OpenGL Context:  %d.%d", major, minor);

#ifdef USE_PROFILER
	rmt_Settings()->enableThreadSampler = RMT_FALSE; // TODO: Investigate assert on exit with this enabled
	rmt_Settings()->reuse_open_port = RMT_TRUE;
	rmt_Settings()->limit_connections_to_localhost = RMT_TRUE;
	rmtError rmtErr = rmt_CreateGlobalInstance(&m_remotery);
	if (rmtErr != RMT_ERROR_NONE)
		logError("Failed to initialize Remotery profiler (code %d)", rmtErr);
	else rmt_BindOpenGL();
#endif
}

void Engine::moduleInit()
{
	s_singleton = this;
#ifdef _WIN32
	if (!gladLoadGL()) {
		panic("Failed to load OpenGL functions");
	}
#endif
#ifdef USE_PROFILER
	rmt_SetGlobalInstance(m_remotery);
#endif
}

void Engine::deinit()
{
#ifdef USE_PROFILER
	rmt_UnbindOpenGL();
	rmt_DestroyGlobalInstance(m_remotery);
#endif
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Engine::swap()
{
	SDL_GL_SwapWindow(window);
	if (m_threadpool.size() != threads)
		m_threadpool.resize(threads);
	Uint64 curTime = SDL_GetPerformanceCounter();
	dtUnadjusted = (curTime - m_prevTime) / (float)SDL_GetPerformanceFrequency();
	dt = timeMult * dtUnadjusted;
	m_prevTime = curTime;
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

void Engine::fullscreen(bool enable)
{
	if (SDL_SetWindowFullscreen(window, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
		logError("Fullscreen %s failed: %s", enable ? "enabling" : "disabling", SDL_GetError());
	else if (enable) {
		SDL_DisplayMode mode;
		if (SDL_GetWindowDisplayMode(window, &mode) == 0) {
			m_width = mode.w;
			m_height = mode.h;
		}
		logInfo("Fullscreen enabled (%dx%d)", m_width, m_height);
	} else {
		m_width = settings["screen"]["width"].int_value();
		m_height = settings["screen"]["height"].int_value();
		logInfo("Fullscreen disabled");
	}
}

bool Engine::fullscreen()
{
	return (SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}

void Engine::setIcon(Image* image)
{
	if (!image) {
		SDL_SetWindowIcon(window, nullptr);
		return;
	}
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image->data.data(), image->width, image->height, image->channels * 8, image->width * image->channels, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_SetWindowIcon(window, surface);
	SDL_FreeSurface(surface);
}

uint Engine::timems()
{
	return SDL_GetTicks();
}

int Engine::width()
{
	ASSERT(s_singleton);
	return s_singleton->m_width;
}

int Engine::height()
{
	ASSERT(s_singleton);
	return s_singleton->m_height;
}

float Engine::deltaTime()
{
	ASSERT(s_singleton);
	return s_singleton->dt;
}

void Engine::grabMouse(bool grab)
{
	SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
}

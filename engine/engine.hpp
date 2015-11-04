#pragma once
#include "common.hpp"

class Engine
{
public:
	Engine();
	~Engine();

	void init(const string& configPath);
	void moduleInit(); // Call in each module's INIT handler
	void deinit();
	void swap();

	void vsync(bool enable);
	bool vsync();
	void fullscreen(bool enable);
	bool fullscreen();

	static int width();
	static int height();

	void grabMouse(bool grab);

	static uint timems();

	float dt = 0.f;
	struct SDL_Window* window = nullptr;

	static Json settings;

private:
	static Engine* s_singleton;
	int m_width = 0;
	int m_height = 0;
	uint64 m_prevTime = 0;
	void* m_glcontext;
};

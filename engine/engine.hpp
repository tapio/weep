#pragma once
#include "common.hpp"

class Engine
{
public:
	static void init(const string& configPath);
	static void deinit();
	static void swap();

	static void vsync(bool enable);
	static bool vsync();
	static void fullscreen(bool enable);
	static bool fullscreen();

	static int width();
	static int height();

	static void grabMouse(bool grab);

	static uint timems();

	static float dt;
	static Json settings;
	static struct SDL_Window* window;
};

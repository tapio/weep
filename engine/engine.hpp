#pragma once
#include "common.hpp"

class Engine
{
public:
	static void init(const string& configPath);
	static void deinit();
	static void swap();

	static int width();
	static int height();

	static void grabMouse(bool grab);

	static float dt;
	static Json settings;
};

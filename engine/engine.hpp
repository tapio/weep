#pragma once
#include "common.hpp"

class Engine
{
public:
	typedef std::function<void()> FrameFunc;

	static void init();
	static void deinit();
	static void swap();

	static int width();
	static int height();
};

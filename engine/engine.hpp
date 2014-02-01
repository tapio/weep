#pragma once
#include "common.hpp"

class Engine
{
public:
	typedef std::function<void()> FrameFunc;

	static void init();
	static void deinit();
	static void run(const FrameFunc& callback);
};

#pragma once
#include "common.hpp"
#include "../../data/shaders/uniforms.glsl"

template<typename T>
struct UBO
{
	UBO() {}
	~UBO();

	NONCOPYABLE(UBO);

	void create();
	void upload();
	void destroy();

	uint id = 0;
	T uniforms = T();
};

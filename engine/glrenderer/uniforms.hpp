#pragma once
#include "common.hpp"
#include "../../data/shaders/uniforms.glsl"

template<typename T>
class UBO
{
public:
	NONCOPYABLE(UBO);

	uint id = 0;
	int binding = 0;
	uint arraySize = 1;
	T uniforms = T();

	UBO() {}

	void create(uint binding);
	void upload();
	void destroy();
};

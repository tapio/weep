#pragma once
#include "common.hpp"
#include "../../data/shaders/uniforms.glsl"

template<typename T>
class UBO
{
public:
	NONCOPYABLE(UBO);

	uint id = 0;
	T uniforms = T();

	UBO() {}
	~UBO();

	void create();
	void upload();
	void destroy();
};

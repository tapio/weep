#pragma once
#include "common.hpp"
#include "../../data/shaders/uniforms.glsl"


// Uniform Buffer Object
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


// Shader Storage Buffer Object
template<typename T>
struct SSBO
{
	SSBO(uint binding): binding(binding) {}
	~SSBO();

	NONCOPYABLE(SSBO);

	void create();
	void upload();
	void destroy();

	uint id = 0;
	uint binding = 0;
	std::vector<T> buffer;
};

#pragma once
#include "common.hpp"

struct UniformCommonBlock {
	mat4 modelMatrix = mat4();
	mat4 modelViewMatrix = mat4();
	mat4 projectionMatrix = mat4();
	mat4 viewMatrix = mat4();
	mat3 normalMatrix = mat3();
	vec3 cameraPosition = vec3();
};

struct UniformColorBlock {
	vec3 ambient = vec3();
	vec3 diffuse = vec3();
	vec3 specular = vec3();
};

struct UniformLightBlock {
	vec3 color = vec3(1, 1, 1);
	vec3 position = vec3();
	vec3 direction = vec3();
	vec3 params = vec3();
};

template<typename T>
class UBO
{
public:
	NONCOPYABLE(UBO);

	uint id = 0;
	int binding = 0;
	T uniforms = T();

	UBO() {}

	void create(uint binding);
	void upload();
	void destroy();
};

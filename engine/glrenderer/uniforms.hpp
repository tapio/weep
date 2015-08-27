#pragma once
#include "common.hpp"

#define MAX_LIGHTS 4u

struct UniformCommonBlock {
	mat4 modelMatrix = mat4();
	mat4 modelViewMatrix = mat4();
	mat4 projectionMatrix = mat4();
	mat4 viewMatrix = mat4();
	mat4 normalMatrix = mat4();
	vec3 cameraPosition = vec3(); float numLights;
};

struct UniformColorBlock {
	vec3 ambient = vec3(); float pad1;
	vec3 diffuse = vec3(); float pad2;
	vec3 specular = vec3(); float pad3;
	float shininess = 0.f;
};

struct UniformLightBlock {
	vec3 color = vec3(1, 1, 1); float pad1;
	vec3 position = vec3(); float pad2;
	vec3 direction = vec3(); float pad3;
	vec3 params = vec3(); float pad4;
};

template<typename T, uint N>
class UBO
{
public:
	NONCOPYABLE(UBO);

	uint id = 0;
	int binding = 0;
	uint arraySize = 1;
	union {
		T uniforms;
		T uniformArray[N] = {};
	};

	UBO() {}

	void create(uint binding);
	void upload();
	void destroy();
};

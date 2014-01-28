#pragma once
#include "common.hpp"

struct Material
{
	vec3 ambient = vec3(0, 0, 0);
	vec3 diffuse = vec3(1, 1, 1);
	vec3 specular = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);

	void* diffuseMap = nullptr;
	void* normalMap = nullptr;
	void* specularMap = nullptr;

	uint shaderId = 0;
};

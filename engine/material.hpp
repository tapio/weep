#pragma once
#include "common.hpp"

struct Image;

struct Material
{
	vec3 ambient = vec3(0, 0, 0);
	vec3 diffuse = vec3(1, 1, 1);
	vec3 specular = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);

	Image* diffuseMap = nullptr;
	Image* normalMap = nullptr;
	Image* specularMap = nullptr;

	uint diffuseTex = 0;
	uint normalTex = 0;
	uint specularTex = 0;

	uint shaderId = 0;
};

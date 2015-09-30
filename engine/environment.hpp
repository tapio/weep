#pragma once
#include "common.hpp"
#include "resources.hpp"


struct Environment
{
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	float exposure = 1.f;
	float bloomThreshold = 0.4f;
	float bloomIntensity = 5.f;
	enum Tonemap {
		TONEMAP_REINHARD,
		TONEMAP_EXPOSURE,
		TONEMAP_FILMIC,
		TONEMAP_UNCHARTED2,
		TONEMAP_COUNT
	} tonemap = TONEMAP_REINHARD;

	vec3 sunDirection = vec3(0, -1, 0);
	vec3 sunColor = vec3(0, 0, 0);

	vec3 fogColor = vec3(0.5, 0.5, 0.5);
	float fogDensity = 0.f;

	struct Image* skybox[6] = {};
};

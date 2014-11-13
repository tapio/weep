#pragma once
#include "common.hpp"

struct Light
{
	Light() {}

	enum Type {
		AMBIENT_LIGHT,
		POINT_LIGHT,
		DIRECTIONAL_LIGHT,
		SPOT_LIGHT,
		AREA_LIGHT,
		HEMISPHERE_LIGHT
	} type = AMBIENT_LIGHT;

	vec3 color = vec3(1, 1, 1);
	vec3 position = vec3();
	vec3 direction = vec3();
	float distance = 0.0f;
};

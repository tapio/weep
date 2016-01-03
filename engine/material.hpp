#pragma once
#include "common.hpp"

struct Image;

enum Technique {
	TECH_COLOR,
	TECH_DEPTH,
	TECH_DEPTH_CUBE,
	NUM_TECHNIQUES
};

struct Material
{
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(1, 1, 1);
	vec3 specular = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);
	float shininess = 32.0f;
	float reflectivity = 0.f;
	vec2 uvOffset = vec2(0, 0);
	vec2 uvRepeat = vec2(1, 1);

	enum MapTypes {
		DIFFUSE_MAP,
		NORMAL_MAP,
		SPECULAR_MAP,
		HEIGHT_MAP,
		EMISSION_MAP,
		AO_MAP,
		REFLECTION_MAP,
		ENV_MAP,
		MAX_MAPS
	};

	Image* map[MAX_MAPS] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	uint tex[MAX_MAPS] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	enum Flags {
		TESSELLATE = 1 << 0,
		DIRTY_MAPS = 1 << 1,
		CAST_SHADOW = 1 << 2,
		RECEIVE_SHADOW = 1 << 3,
		ANIMATED = 1 << 4,
		ALPHA_TEST = 1 << 5
	};
	uint flags = DIRTY_MAPS | CAST_SHADOW | RECEIVE_SHADOW;

	int shaderId[NUM_TECHNIQUES] = { -1, -1, -1 }; // Automatic
	string shaderName = "";
};

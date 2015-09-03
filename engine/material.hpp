#pragma once
#include "common.hpp"

struct Image;

struct Material
{
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(1, 1, 1);
	vec3 specular = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);
	float shininess = 32.0f;
	vec2 uvOffset = vec2(0, 0);
	vec2 uvRepeat = vec2(1, 1);

	enum MapTypes {
		DIFFUSE_MAP,
		NORMAL_MAP,
		SPECULAR_MAP,
		HEIGHT_MAP,
		EMISSION_MAP,
		REFLECTION_MAP,
		ENV_MAP,
		MAX_MAPS
	};

	std::vector<Image*> map = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	std::vector<uint> tex = { 0, 0, 0, 0, 0, 0, 0 };

	bool tessellate = false;
	string shaderName = "";
	int shaderId = -1; // Automatic
};

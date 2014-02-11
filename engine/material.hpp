#pragma once
#include "common.hpp"

struct Image;

struct Material
{
	vec3 ambient = vec3(0, 0, 0);
	vec3 diffuse = vec3(1, 1, 1);
	vec3 specular = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);

	enum MapTypes {
		DIFFUSE_MAP, NORMAL_MAP, SPECULAR_MAP, HEIGHT_MAP, MAX_MAPS
	};

	std::vector<Image*> map = { nullptr, nullptr, nullptr, nullptr };
	std::vector<uint> tex = { 0, 0, 0, 0 };

	bool tessellate = false;
	string shaderName = "";
	int shaderId = -1; // Automatic
};

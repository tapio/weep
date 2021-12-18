#pragma once
#include "common.hpp"

struct Image;

enum Technique {
	TECH_COLOR,
	TECH_REFLECTION,
	TECH_DEPTH,
	TECH_DEPTH_CUBE,
	TECH_COMPUTE,
	NUM_TECHNIQUES
};

struct Material
{
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(1, 1, 1);
	vec3 specular = vec3(1, 1, 1);
	vec3 emissive = vec3(0, 0, 0);
	float metalness = 0.f;
	float roughness = 0.5f;
	float shininess = 0.f; // Blinn-Phong lighting model
	float reflectivity = 0.f;
	float parallax = 0.f;
	float alphaTest = 0.f;
	vec2 uvOffset = vec2(0, 0);
	vec2 uvRepeat = vec2(1, 1);
	vec2 particleSize = vec2(0.01f, 0.01f);

	enum BlendFunc {
		BLEND_NONE,
		BLEND_ALPHA,
		BLEND_ADD,
		BLEND_SUBTRACT
	} blendFunc = BLEND_NONE;

	enum LightingModel {
		LIGHTING_MODEL_NONE,
		LIGHTING_MODEL_AUTO, // Select based on other parameters
		//LIGHTING_MODEL_PHONG,
		LIGHTING_MODEL_BLINN_PHONG,
		LIGHTING_MODEL_PBR
	} lightingModel = LIGHTING_MODEL_AUTO;

	enum MapTypes {
		DIFFUSE_MAP,
		NORMAL_MAP,
		SPECULAR_MAP,
		EMISSION_MAP,
		HEIGHT_MAP,
		ROUGHNESS_MAP,
		METALNESS_MAP,
		AO_MAP,
		REFLECTION_MAP,
		ENV_MAP,
		MAX_MAPS
	};

	Image* map[MAX_MAPS] = { };
	uint tex[MAX_MAPS] = { };

	enum Flags {
		TESSELLATE = 1 << 0,
		DIRTY_MAPS = 1 << 1,
		CAST_SHADOW = 1 << 2,
		RECEIVE_SHADOW = 1 << 3,
		ANIMATED = 1 << 4,
		DRAW_REFLECTION = 1 << 5,
	};
	uint flags = DIRTY_MAPS | CAST_SHADOW | RECEIVE_SHADOW | DRAW_REFLECTION;

	int shaderId[NUM_TECHNIQUES] = { -1, -1, -1, -1, -1 }; // Automatic
	string shaderName = "";
};

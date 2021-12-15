#pragma once
#include "common.hpp"
#include "resources.hpp"


struct Environment
{
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	float shadowDarkness = 0.01f;
	float exposure = 1.f;
	float bloomThreshold = 0.4f;
	float bloomIntensity = 5.f;
	enum Tonemap {
		TONEMAP_REINHARD,
		TONEMAP_EXPOSURE,
		TONEMAP_FILMIC,
		TONEMAP_UNCHARTED2,
		TONEMAP_ACES,
		TONEMAP_COUNT
	} tonemap = TONEMAP_REINHARD;

	enum SkyType {
		SKY_SKYBOX,
		SKY_PROCEDURAL,
		SKY_COUNT
	} skyType = SKY_PROCEDURAL;

	vec3 sunPosition = up_axis;
	vec3 sunColor = vec3(0, 0, 0);

	vec3 fogColor = vec3(0.5, 0.5, 0.5);
	float fogDensity = 0.f;

	// TODO: Probably put to some other "PostEffects" class?
	vec3 vignette = vec3(0.5, 0.5, 0.0); // Radius, smoothness, strength
	float saturation = 0.f;
	float sepia = 0.f;
	float chromaticAberration = 0.f;
	float scanlines = 0.f;
	enum PostProcessAA {
		POST_AA_NONE,
		POST_AA_FXAA,
		POST_AA_COUNT
	} postAA;

	struct Image* skybox[6] = {};
};

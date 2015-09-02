#pragma once
#include "common.hpp"
#include "resources.hpp"


struct Environment
{
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	// 0 = Reinhard tonemapping
	float exposure = 0.f;

	struct Image* skybox[6] = {};
};

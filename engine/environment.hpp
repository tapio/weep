#pragma once
#include "common.hpp"

struct Environment
{

	vec3 ambient = vec3(0.1, 0.1, 0.1);
	// 0 = Reinhard tonemapping
	float exposure = 0.f;
};

#pragma once
#include "common.hpp"

struct Camera
{

	void makePerspective(float fov, float aspect, float near, float far)
	{
		projection = perspective(fov, aspect, near, far);
	}

	void makeOrtho(float left, float right, float top, float bottom, float near, float far)
	{
		projection = ortho(left, right, bottom, top, near, far);
	}

	mat4 projection = mat4();
	mat4 view = mat4();
};

#pragma once
#include "common.hpp"

struct Camera
{

	void makePerspective(float fov, float aspect, float near, float far)
	{
		projection = glm::perspective(fov, aspect, near, far);
		this->far = far;
	}

	void makeOrtho(float left, float right, float top, float bottom, float near, float far)
	{
		projection = glm::ortho(left, right, bottom, top, near, far);
		this->far = far;
	}

	void updateViewMatrix() {
		view = glm::mat4_cast(glm::inverse(rotation));
		view = glm::translate(view, -position);
	}

	mat4 projection = mat4();
	mat4 view = mat4();
	vec3 position = vec3();
	quat rotation = quat();
	float far = 0;
};

#pragma once
#include "common.hpp"

#ifdef _WIN32
// Windows defines these to nothing for some ancient compat reasons...
#undef near
#undef far
#endif


struct Camera
{

	void makePerspective(float fov, float aspect, float near, float far)
	{
		projection = glm::perspective(fov, aspect, near, far);
		this->near = near;
		this->far = far;
	}

	void makeOrtho(float left, float right, float top, float bottom, float near, float far)
	{
		projection = glm::ortho(left, right, bottom, top, near, far);
		this->near = near;
		this->far = far;
	}

	void updateViewMatrix(vec3 position, quat rotation) {
		view = glm::mat4_cast(glm::inverse(rotation));
		view = glm::translate(view, -position);
	}

	void updateViewMatrix(vec3 position) {
		view = glm::translate(view, -position);
	}

	vec3 position() const { return -view[3] * view; }

	mat4 projection = mat4();
	mat4 view = mat4();
	float near = 0;
	float far = 0;
};

#pragma once
#include "common.hpp"

class Camera
{
public:
	Camera() {}
	Camera(const Json& json)
	{
		if (json["fov"].is_number()) makePerspective(
			json["fov"].number_value(),
			json["aspect"].number_value(),
			json["near"].number_value(),
			json["far"].number_value());
		else makeOrtho(
			json["left"].number_value(),
			json["right"].number_value(),
			json["top"].number_value(),
			json["bottom"].number_value(),
			json["near"].number_value(),
			json["far"].number_value());
	}

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

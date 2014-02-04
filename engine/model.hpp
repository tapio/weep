#pragma once
#include "common.hpp"

struct Geometry;
struct Material;

struct Model
{
	vec3 position = vec3();
	quat rotation = quat();
	vec3 scale = vec3(1, 1, 1);
	mat4 transform = mat4();

	void updateMatrix() {
		transform = translate(mat4(), position);
		transform = glm::scale(transform, scale);
		transform *= mat4_cast(rotation);
	}

	Geometry* geometry = nullptr;
	std::shared_ptr<Material> material;
};

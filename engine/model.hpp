#pragma once
#include "common.hpp"
#include "geometry.hpp"

struct Material;

struct Transform
{
	vec3 position = vec3();
	quat rotation = quat();
	vec3 scale = vec3(1, 1, 1);
	mat4 matrix = mat4();
	bool dirty = false;

	vec3& setPosition(vec3 pos) { dirty = true; return position = pos; }
	quat& setRotation(quat rot) { dirty = true; return rotation = rot; }
	vec3& setScale(vec3 s)      { dirty = true; return scale = s; }

	void updateMatrix() {
		matrix = glm::translate(position);
		matrix = glm::scale(matrix, scale);
		matrix *= glm::mat4_cast(rotation);
	}
};

struct Model
{
	Bounds bounds;
	Geometry* geometry = nullptr;
	std::vector<Material*> materials;
};

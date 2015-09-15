#pragma once
#include "common.hpp"
#include "geometry.hpp"

struct Material;
class btRigidBody;

struct Model
{
	vec3 position = vec3();
	quat rotation = quat();
	vec3 scale = vec3(1, 1, 1);
	mat4 transform = mat4();

	Bounds bounds;

	void updateMatrix() {
		transform = glm::translate(position);
		transform = glm::scale(transform, scale);
		transform *= glm::mat4_cast(rotation);
	}

	string name = "";
	Geometry* geometry = nullptr;
	std::vector<Material*> materials;
	btRigidBody* body = nullptr; // TODO: Does not belong here
};

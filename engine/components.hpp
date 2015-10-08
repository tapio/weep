#pragma once
#include "common.hpp"

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

struct Bounds
{
	vec3 min = vec3(INFINITY, INFINITY, INFINITY);
	vec3 max = vec3(INFINITY, INFINITY, INFINITY);
	float radius = INFINITY;
};

struct Model
{
	Bounds bounds;
	struct Geometry* geometry = nullptr;
	std::vector<struct Material*> materials;
};

struct GroundTracker
{
	bool onGround = false;
};

struct Light
{
	Light() {}

	enum Type {
		AMBIENT_LIGHT,
		POINT_LIGHT,
		DIRECTIONAL_LIGHT,
		SPOT_LIGHT,
		AREA_LIGHT,
		HEMISPHERE_LIGHT
	} type = AMBIENT_LIGHT;

	vec3 color = vec3(1, 1, 1);
	vec3 position = vec3();
	vec3 direction = vec3();
	float distance = 1.0f;
	float decay = 1.0f;
};


struct MoveSound
{
	string event = "";
	vec3 prevPos = vec3();
	float delta = 0.0f;
	float stepLength = 0.5f;
	bool needsGroundContact = true;
};
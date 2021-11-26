#pragma once
#include "common.hpp"
#include "material.hpp"

struct Transform
{
	vec3 position = vec3();
	quat rotation = quat_identity;
	vec3 scale = vec3(1, 1, 1);
	mat4 matrix = mat4();
	bool dirty = false;

	vec3& setPosition(vec3 pos) { dirty = true; return position = pos; }
	quat& setRotation(quat rot) { dirty = true; return rotation = rot; }
	vec3& setScale(vec3 s)      { dirty = true; return scale = s; }

	void updateMatrix() {
		matrix = glm::translate(mat4(1.f), position);
		matrix *= glm::mat4_cast(rotation);
		matrix = glm::scale(matrix, scale);
	}
};

struct Bounds
{
	vec3 min = vec3(INFINITY, INFINITY, INFINITY);
	vec3 max = vec3(INFINITY, INFINITY, INFINITY);
	float radius = INFINITY;
};

struct BoneAnimation
{
	std::vector<mat3x4> bones;
	enum State {
		STOPPED, PLAYING, PAUSED
	} state = STOPPED;
	uint animation = 0;
	float time = 0.f;
	float speed = 1.f;
};

struct Geometry;

struct Model
{
	struct Lod {
		float distSq = FLT_MAX;
		Geometry* geometry = nullptr;
	};
	static const int MAX_LODS = 3;
	Lod lods[MAX_LODS] = {};

	Geometry* getLod(float distance) {
		return getLod2(distance * distance);
	}

	Geometry* getLod2(float distanceSq) {
		for (int i = 0; i < MAX_LODS; ++i)
			if (distanceSq < lods[i].distSq)
				return lods[i].geometry;
		return nullptr;
	}

	Bounds bounds;
	Geometry* geometry = nullptr; // Current LOD
	std::vector<Material> materials;
};

struct GroundTracker
{
	bool onGround = false;
};

struct ContactTracker
{
	bool hadContact = false;
};

struct TriggerVolume
{
	uint times = -1;
	uint groups = 0;
	Bounds bounds;
	uint receiverModule = 0;
	uint enterMessage = 0;
	uint exitMessage = 0;
};

struct TriggerGroup
{
	uint group = 0;
	bool triggered = false;
};

struct Light
{
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
	vec3 target = vec3();
	float distance = 1.0f;
	float decay = 1.0f;
	float priority = 0.f;
};


struct MoveSound
{
	vec3 prevPos = vec3();
	float delta = 0.0f;
	float stepLength = 0.5f;
	uint event = 0;
	bool needsGroundContact = true;
};

struct ContactSound
{
	uint event = 0;
};

struct DebugInfo
{
	string name = "";
};

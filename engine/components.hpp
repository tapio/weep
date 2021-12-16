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

	Transform& modify() { dirty = true; return *this; }

	Transform& setPosition(vec3 pos) { dirty = true; position = pos; return *this; }
	Transform& setRotation(quat rot) { dirty = true; rotation = rot; return *this; }
	Transform& setScale(vec3 s)      { dirty = true; scale = s; return *this; }

	Transform& translate(vec3 offset) { dirty = true; position += offset; return *this; }

	vec3 forward() const { return rotation * forward_axis; }

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

enum class AnimationState : uint {
	STOPPED, PLAYING, PAUSED
};

enum class AnimationMode : uint {
	ONCE, LOOP, PINGPONG
};

struct BoneAnimation
{
	std::vector<mat3x4> bones;
 	AnimationState state = AnimationState::STOPPED;
	uint animation = 0;
	float time = 0.f;
	float speed = 1.f;
};

struct PropertyAnimation
{
	template<typename T>
	struct Keyframe
	{
		float time = 0.f;
		T value = {};
	};

	template<typename T>
	struct Track
	{
		Track(uint id, const std::vector<Keyframe<T>>& keyframes): id(id), keyframes(keyframes) {}
		T currentValue = {};
		uint id = 0;
		std::vector<Keyframe<T>> keyframes;
		float length() const { return keyframes.empty() ? 0.f : keyframes.back().time; }
	};

	AnimationState state = AnimationState::STOPPED;
	AnimationMode mode = AnimationMode::LOOP;
	float time = 0.f;
	float speed = 1.f;
	float length = 0.f;
	std::vector<Track<float>> floatTracks;
	std::vector<Track<vec3>> vec3Tracks;
	std::vector<Track<quat>> quatTracks;
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

struct Particles
{
	uint count = 0;
	uint computeId = 0;
	int renderId = -1;
	Bounds bounds;
	Material material;

	bool emit = true;
	bool localSpace = false;
	float directionality = 0.f;
	float randomRotation = 0.f;
	vec2 emitRadiusMinMax = vec2(0.f, 0.f);
	vec2 lifeTimeMinMax = vec2(0.f, 1.f);
	vec2 speedMinMax = vec2(0.1f, 1.f);
};

struct RigidBody
{
	class btRigidBody* body = nullptr;
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
	enum Type { // Keep defines in uniforms.glsl in sync!
		POINT_LIGHT,
		DIRECTIONAL_LIGHT,
		SPOT_LIGHT,
	} type = POINT_LIGHT;

	vec3 color = vec3(1, 1, 1);
	vec2 spotAngles = vec2(10.f, 20.f); // Degrees
	float distance = 1.0f;
	float shadowDistance = -1.f;
	float decay = 1.0f;
	float priority = 0.f;

private:
	friend class RenderSystem;
	friend class RenderDevice;

	vec3 position = vec3();
	vec3 direction = vec3();
	int shadowIndex = -1;
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

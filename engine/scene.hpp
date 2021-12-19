#pragma once
#include "common.hpp"
#define ECS_ASSERT ASSERT
#include <ecs/ecs.hpp>

class Resources;

class SceneLoader
{
public:
	SceneLoader() {}
	SceneLoader(ecs::Entities& entities): world(&entities) {}

	void load(const string& path, Resources& resources);
	void reset();

	ecs::Entity instantiate(Json def, Resources& resources, const string& pathContext = "");

	ecs::Entities* world = nullptr;
	std::map<string, Json> prefabs;

private:
	void load_internal(const string& path, Resources& resources);

	Json m_environment;
	uint numModels = 0, numBodies = 0, numLights = 0, numParticles = 0;
};

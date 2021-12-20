#pragma once
#include "common.hpp"
#include <json11/json11.hpp>
#include <ecs/ecs.hpp>

class Resources;

class SceneLoader
{
public:
	SceneLoader() {}
	SceneLoader(ecs::Entities& entities): world(&entities) {}

	void load(const string& path, Resources& resources);
	void reset();

	ecs::Entity instantiate(json11::Json def, Resources& resources, const string& pathContext = "");

	ecs::Entities* world = nullptr;
	std::map<string, json11::Json> prefabs;

private:
	void load_internal(const string& path, Resources& resources);

	json11::Json m_environment;
	uint numModels = 0, numBodies = 0, numLights = 0, numParticles = 0;
};

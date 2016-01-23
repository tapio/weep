#pragma once
#include "common.hpp"

class Resources;

class SceneLoader
{
public:
	SceneLoader() {}
	SceneLoader(Entities& entities): world(&entities) {}

	void load(const string& path, Resources& resources);
	void reset();

	Entity instantiate(Json def, Resources& resources);

	Entities* world = nullptr;
	std::map<string, Json> prefabs;

private:
	void load_internal(const string& path, Resources& resources);

	Json m_environment;
	uint numModels = 0, numBodies = 0, numLights = 0;
};

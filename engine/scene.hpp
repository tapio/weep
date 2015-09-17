#pragma once
#include "common.hpp"
#include "model.hpp"
#include "light.hpp"

class Resources;

class Scene
{
public:

	void load(const string& path, Resources& resources);
	void reset();

	Entities world;

private:
	void load_internal(const string& path, Resources& resources);

	uint numModels = 0, numBodies = 0, numLights = 0;
	std::map<string, Json> m_prefabs;
};

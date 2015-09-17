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
	std::map<string, Json> m_prefabs;
};

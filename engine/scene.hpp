#pragma once
#include "common.hpp"
#include "model.hpp"
#include "light.hpp"
#include "entity/world.hpp"

class Resources;

class Scene
{
public:

	void load(const string& path, Resources& resources);
	void reset();

	std::vector<Model>& getChildren() { return m_models; }
	std::vector<Light>& getLights() { return m_lights; }

	Model* find(const string& name);

	entity::World world;

private:
	std::vector<Model> m_models;
	std::vector<Light> m_lights;
	std::map<string, Json> m_prefabs;
};

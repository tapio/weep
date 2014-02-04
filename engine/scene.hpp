#pragma once
#include "common.hpp"
#include "model.hpp"

class Resources;

class Scene
{
public:

	void load(const string& path, Resources& resources);
	void reset();

	std::vector<Model>& getChildren() { return m_models; }

private:
	std::vector<Model> m_models;
};

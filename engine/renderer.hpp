#pragma once
#include "common.hpp"

class RenderDevice;
struct Camera;
struct Model;

class Renderer
{
public:
	Renderer();
	~Renderer();

	void addModel(Model* model);
	void render(Camera& camera);
	void reset();

private:
	RenderDevice* m_device = nullptr;
	std::vector<Model*> m_models;
};

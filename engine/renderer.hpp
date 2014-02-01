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

private:
	RenderDevice* device = nullptr;
	std::vector<Model*> models;
};

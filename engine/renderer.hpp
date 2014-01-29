#pragma once
#include "common.hpp"

class RenderDevice;
struct Model;

class Renderer
{
public:
	static void create();
	static void destroy();

	void addModel(Model* model);
	void render();

private:
	friend Renderer& GetRenderer();
	Renderer();
	~Renderer();

	RenderDevice* device = nullptr;
	std::vector<Model*> models;

	static Renderer* instance;
};


inline Renderer& GetRenderer() {
	return *Renderer::instance;
}

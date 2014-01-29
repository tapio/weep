#pragma once
#include "common.hpp"

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

	std::vector<Model*> models;

	static Renderer* instance;
};


inline Renderer& GetRenderer() {
	return *Renderer::instance;
}

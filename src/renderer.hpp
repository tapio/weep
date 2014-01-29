#pragma once
#include "common.hpp"

struct Model;

class Renderer
{
public:
	void addModel(std::shared_ptr<Model> model);
	void render();

private:
	friend Renderer& GetRenderer();
	Renderer();
	~Renderer();

	std::vector<std::shared_ptr<Model>> models;
};


inline Renderer& GetRenderer() {
	static Renderer s_renderer;
	return s_renderer;
}

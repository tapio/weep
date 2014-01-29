#include "renderer.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "glrenderer/renderdevice.hpp"

Renderer* Renderer::instance = nullptr;
void Renderer::create() { instance = new Renderer(); }
void Renderer::destroy() { if (instance) delete instance; }


Renderer::Renderer()
{
	device = new RenderDevice();
}

Renderer::~Renderer()
{
	for (auto model : models) {
		device->destroyModel(*model);
	}
}

void Renderer::addModel(Model* model)
{
	device->uploadModel(*model);
	models.push_back(model);
}

void Renderer::render()
{
	device->preRender();
	for (auto model : models) {
		device->render(*model);
	}
	device->postRender();
}



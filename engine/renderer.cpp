#include "renderer.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "glrenderer/renderdevice.hpp"

Renderer::Renderer()
{
	device = new RenderDevice();
}

Renderer::~Renderer()
{
	for (auto model : models) {
		device->destroyModel(*model);
	}
	delete device;
}

void Renderer::addModel(Model* model)
{
	device->uploadModel(*model);
	models.push_back(model);
}

void Renderer::render(Camera& camera)
{
	device->preRender(camera);
	for (auto model : models) {
		device->render(*model);
	}
	device->postRender();
}



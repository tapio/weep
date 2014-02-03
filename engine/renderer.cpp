#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "model.hpp"
#include "camera.hpp"

Renderer::Renderer()
{
	m_device = new RenderDevice();
}

Renderer::~Renderer()
{
	for (auto model : m_models) {
		m_device->destroyModel(*model);
	}
	delete m_device;
}

void Renderer::addModel(Model* model)
{
	m_device->uploadModel(*model);
	m_models.push_back(model);
}

void Renderer::render(Camera& camera)
{
	camera.updateViewMatrix();
	m_device->preRender(camera);
	for (auto model : m_models) {
		m_device->render(*model);
	}
	m_device->postRender();
}



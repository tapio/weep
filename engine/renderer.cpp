#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "scene.hpp"

Renderer::Renderer()
{
	m_device = new RenderDevice();
}

Renderer::~Renderer()
{
	//reset();
	delete m_device;
}

void Renderer::reset(Scene& scene)
{
	logDebug("Reseting renderer");
	for (auto& model : scene.getChildren()) {
		m_device->destroyModel(model);
	}
	m_device->loadShaders();
}

void Renderer::render(Scene& scene, Camera& camera)
{
	camera.updateViewMatrix();
	m_device->preRender(camera);
	for (auto& model : scene.getChildren()) {
		m_device->render(model);
	}
	m_device->postRender();
}

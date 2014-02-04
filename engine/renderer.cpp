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
	reset();
	delete m_device;
}

void Renderer::reset()
{
	//for (auto model : m_models) {
	//	m_device->destroyModel(*model);
	//}
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

#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "image.hpp"


class Frustum
{
public:
	Frustum(const Camera& camera) {
		m_radius = camera.far * 0.5f;
		vec3 dir = camera.rotation * vec3(0, 0, -1);
		m_center = camera.position + dir * m_radius;
	}

	bool visible(const Model& model) {
		float maxDist = model.bounds.radius + m_radius;
		return glm::distance2(m_center, model.position) < maxDist * maxDist;
	}

private:
	float m_radius;
	vec3 m_center;
};



RenderSystem::RenderSystem(Resources& resources)
{
	m_device.reset(new RenderDevice(resources));

	m_env.load("environment.json", resources);
	m_device->setEnvironment(&m_env);
}

RenderSystem::~RenderSystem()
{
	//reset();
}

void RenderSystem::reset(SceneLoader& scene)
{
	logDebug("Reseting renderer");
	scene.world->for_each<Model>([this](Entity, Model& model) {
		m_device->destroyModel(model);
	});
}

void RenderSystem::dumpStats() const
{
	RenderDevice::Stats& stats = m_device->stats;
	logDebug("RenderDevice frame stats:\n"
		"\ttriangles:  %d\n"
		"\tprograms:   %d\n"
		"\tdraw calls: %d",
		stats.triangles, stats.programs, stats.drawCalls);
}

void RenderSystem::toggleWireframe()
{
	m_device->toggleWireframe();
}

void RenderSystem::render(SceneLoader& scene, Camera& camera)
{
	camera.updateViewMatrix();
	Frustum frustum(camera);
	std::vector<Light> lights;
	scene.world->for_each<Light>([&](Entity, Light& light) {
		lights.push_back(light);
	});
	m_device->preRender(camera, lights);
	scene.world->for_each<Model>([&](Entity, Model& model) {
		if (!model.materials.empty() && frustum.visible(model))
			m_device->render(model);
	});
	m_device->postRender();
}

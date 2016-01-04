#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "components.hpp"
#include "geometry.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "image.hpp"
#include <algorithm>


class Frustum
{
public:
	Frustum(const Camera& camera) {
		m_radius = camera.far * 0.5f;
		vec3 dir = camera.rotation * vec3(0, 0, -1);
		m_center = camera.position + dir * m_radius;
	}

	bool visible(const Transform& transform, const Model& model) {
		float maxDist = model.bounds.radius + m_radius;
		return glm::distance2(m_center, transform.position) < maxDist * maxDist;
	}

private:
	float m_radius;
	vec3 m_center;
};



RenderSystem::RenderSystem(Resources& resources)
{
	m_device.reset(new RenderDevice(resources));
	m_device->setEnvironment(&m_env);
}

RenderSystem::~RenderSystem()
{
	//reset();
}

void RenderSystem::reset(Entities& entities)
{
	logDebug("Reseting renderer");
	entities.for_each<Model>([this](Entity, Model& model) {
		if (model.geometry)
			m_device->destroyGeometry(*model.geometry);
	});
}

void RenderSystem::toggleWireframe()
{
	m_device->toggleWireframe();
}

void RenderSystem::render(Entities& entities, Camera& camera)
{
	m_device->stats = RenderDevice::Stats();
	camera.updateViewMatrix();

	entities.for_each<Model, Transform>([&](Entity, Model&, Transform& transform) {
		transform.updateMatrix();
	});

	Frustum frustum(camera);

	std::vector<Light> lights;
	// TODO: Better prioritizing
	vec3 lightTarget = camera.position + camera.rotation * vec3(0, 0, -2);
	entities.for_each<Light>([&](Entity, Light& light) {
		light.priority = glm::distance2(lightTarget, light.position);
		lights.push_back(light);
	});
	std::sort(lights.begin(), lights.end(), [](const Light& a, const Light& b) {
		return a.priority < b.priority;
	});

	// Fixed amount of time for uploading each frame?
	entities.for_each<Model>([&](Entity, Model& model) {
		// Upload geometry
		Geometry& geom = *model.geometry;
		if (!geom.batches.empty() && geom.batches.front().renderId < 0)
			m_device->uploadGeometry(geom);
		// Upload materials
		for (auto mat : model.materials)
			if (mat->shaderId[0] < 0 || (mat->flags & Material::DIRTY_MAPS))
				m_device->uploadMaterial(*mat);
	});

	Light sun;
	sun.type = Light::DIRECTIONAL_LIGHT;
	sun.position = camera.position + normalize(m_env.sunPosition) * 10.f;
	sun.target = camera.position;
	m_device->setupShadowPass(sun, 0);
	if (settings.shadows) {
		entities.for_each<Model, Transform>([&](Entity, Model& model, Transform& transform) {
			// TODO: Shadow frustum culling
			if (!model.materials.empty() /* && frustum.visible(transform, model) */)
				m_device->renderShadow(model, transform);
		});
	}

	// TODO: Account for non-point lights
	uint numCubeShadows = std::min((uint)lights.size(), (uint)MAX_SHADOW_CUBES);
	for (uint i = 0; i < numCubeShadows; ++i) {
		Light& light = lights[i];
		m_device->setupShadowPass(light, 1+i);
		if (settings.shadows) {
			entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
				if (model.materials.empty())
					return;
				float maxDist = model.bounds.radius + light.distance;
				if (glm::distance2(light.position, transform.position) < maxDist * maxDist)
					m_device->renderShadow(model, transform, e.has<Animation>() ? &e.get<Animation>() : nullptr);
			});
		}
	}

	Camera reflCam = camera;
	reflCam.makePerspective(glm::radians(90.0f), 1.f, 0.25f, 50.f);
	m_device->setupRenderPass(reflCam, lights, TECH_REFLECTION);
	entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
		float maxDist = model.bounds.radius + reflCam.far;
		if (!model.materials.empty() && glm::distance2(reflCam.position, transform.position) < maxDist * maxDist)
			m_device->render(model, transform, e.has<Animation>() ? &e.get<Animation>() : nullptr);
	});

	m_device->setupRenderPass(camera, lights, TECH_COLOR);
	entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
		if (!model.materials.empty() && frustum.visible(transform, model))
			m_device->render(model, transform, e.has<Animation>() ? &e.get<Animation>() : nullptr);
	});
	m_device->postRender();
}

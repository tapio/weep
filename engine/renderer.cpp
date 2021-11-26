#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "components.hpp"
#include "geometry.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "image.hpp"
#include <algorithm>
#include <glm/gtx/component_wise.hpp>

class Frustum
{
public:
	Frustum(const Camera& camera, vec3 position, quat rotation) {
		m_radius = camera.far * 0.5f;
		vec3 dir = rotation * vec3(0, 0, -1);
		m_center = position + dir * m_radius;
	}

	bool visible(const Transform& transform, const Model& model) const {
		float maxDist = model.bounds.radius * glm::compMax(transform.scale) + m_radius;
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
		for (int i = 0; i < Model::MAX_LODS && model.lods[i].geometry; ++i)
			m_device->destroyGeometry(*model.lods[i].geometry);
	});
}

void RenderSystem::toggleWireframe()
{
	m_device->toggleWireframe();
}

void RenderSystem::render(Entities& entities, Camera& camera, const Transform& camTransform)
{
	m_device->stats = RenderDevice::Stats();

	bool cubeShadows = settings.shadows;
	if (!m_device->caps.geometryShaders || !m_device->caps.cubeFboAttachment)
	{
		cubeShadows = false;
		settings.dynamicReflections = false;
	}

	struct ReflectionProbe { float priority; vec3 pos; };
	std::vector<ReflectionProbe> reflectionProbes;
	std::vector<Light> lights;

	START_MEASURE(prerenderMs)
	vec3 camPos = camTransform.position;
	quat camRot = camTransform.rotation;
	camera.updateViewMatrix(camPos, camRot);
	Frustum frustum(camera, camPos, camRot);

	entities.for_each<Model, Transform>([&](Entity, Model& model, Transform& transform) {
		// Update transform
		transform.updateMatrix();
		// Update LOD
		#ifdef SHIPPING_BUILD
		model.geometry = model.getLod2(glm::distance2(camTransform.position, transform.position));
		#else
		model.geometry = settings.forceLod >= 0 ? model.lods[settings.forceLod].geometry
			: model.getLod2(glm::distance2(camPos, transform.position));
		#endif
	});

	if (settings.dynamicReflections) {
		entities.for_each<Model, Transform>([&](Entity, Model& model, Transform& transform) {
			// Figure out candidates for reflection location
			if (frustum.visible(transform, model)) {
				float reflectivity = 0.f;
				for (auto& mat : model.materials)
					if (mat.reflectivity > reflectivity)
						reflectivity = mat.reflectivity;
				if (reflectivity > 0.01f) {
					float priority = glm::distance2(camPos, transform.position);
					priority *= 1.1f - reflectivity;
					reflectionProbes.push_back({ priority, transform.position });
				}
			}
		});
		std::sort(reflectionProbes.begin(), reflectionProbes.end(), [](const ReflectionProbe& a, const ReflectionProbe& b) {
			return a.priority < b.priority;
		});
	}

	// TODO: Better prioritizing
	vec3 lightTarget = camPos + camRot * vec3(0, 0, -2);
	entities.for_each<Light>([&](Entity, Light& light) {
		light.priority = glm::distance2(lightTarget, light.position);
		lights.push_back(light);
	});
	std::sort(lights.begin(), lights.end(), [](const Light& a, const Light& b) {
		return a.priority < b.priority;
	});

	END_MEASURE(prerenderMs)

	// Fixed amount of time for uploading each frame?
	START_MEASURE(uploadMs)
	BEGIN_GPU_SAMPLE(Upload)
	int uploadCount = 0;
	entities.for_each<Model>([&](Entity, Model& model) {
		// Upload geometries
		for (int i = 0; i < Model::MAX_LODS && model.lods[i].geometry; ++i) {
			Geometry& geom = *model.lods[i].geometry;
			if (!geom.batches.empty() && geom.batches.front().renderId < 0) {
				m_device->uploadGeometry(geom);
				++uploadCount;
			}
		}
		// Upload materials
		for (auto& mat : model.materials) {
			if (mat.shaderId[0] < 0 || (mat.flags & Material::DIRTY_MAPS)) {
				m_device->uploadMaterial(mat);
				++uploadCount;
			}
		}
	});
	END_GPU_SAMPLE()
	END_MEASURE(uploadMs)
	if (uploadCount > 0)
		logDebug("Async GPU upload operations count %d in %.1fms (cpu)", uploadCount, uploadMs);

	START_MEASURE(shadowMs)
	BEGIN_GPU_SAMPLE(ShadowPass)
	Light sun;
	sun.type = Light::DIRECTIONAL_LIGHT;
	sun.position = camPos + normalize(m_env.sunPosition) * 10.f;
	sun.target = camPos;
	m_device->setupShadowPass(sun, 0);
	if (settings.shadows) {
		entities.for_each<Model, Transform>([&](Entity, Model& model, Transform& transform) {
			// TODO: Shadow frustum culling
			if (!model.materials.empty() && model.geometry /* && frustum.visible(transform, model) */)
				m_device->renderShadow(model, transform);
		});
	}

	// TODO: Account for non-point lights
	if (cubeShadows) {
		uint numCubeShadows = std::min((uint)lights.size(), (uint)MAX_SHADOW_CUBES);
		for (uint i = 0; i < numCubeShadows; ++i) {
			Light& light = lights[i];
			m_device->setupShadowPass(light, 1+i);
			if (settings.shadows) {
				entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
					if (model.materials.empty() || !model.geometry)
						return;
					float maxDist = model.bounds.radius * glm::compMax(transform.scale) + light.distance;
					if (glm::distance2(light.position, transform.position) < maxDist * maxDist)
						m_device->renderShadow(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr);
				});
			}
		}
	}
	END_GPU_SAMPLE()
	END_MEASURE(shadowMs)

	// Reflection pass
	START_MEASURE(reflectionMs)
	BEGIN_GPU_SAMPLE(ReflectionPass)
	if (settings.dynamicReflections) {
		Camera reflCam;
		reflCam.makePerspective(glm::radians(90.0f), 1.f, 0.1f, 50.f);
		vec3 reflCamPos = reflectionProbes.empty() ? camPos : reflectionProbes.front().pos;
		reflCam.updateViewMatrix(reflCamPos);
		m_device->setupRenderPass(reflCam, lights, TECH_REFLECTION);
		entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
			float maxDist = model.bounds.radius * glm::compMax(transform.scale) + reflCam.far;
			if (!model.materials.empty() && model.geometry && glm::distance2(reflCamPos, transform.position) < maxDist * maxDist)
				m_device->render(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr);
		});
		m_device->renderSkybox();
	}
	END_GPU_SAMPLE()
	END_MEASURE(reflectionMs)

	// Scene color pass
	START_MEASURE(sceneMs)
	BEGIN_GPU_SAMPLE(ScenePass)
	m_device->setupRenderPass(camera, lights, TECH_COLOR);
	entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
		if (!model.materials.empty() && model.geometry && frustum.visible(transform, model))
			m_device->render(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr);
	});
	m_device->renderSkybox();
	END_GPU_SAMPLE()
	END_MEASURE(sceneMs)

	START_MEASURE(postprocessMs)
	BEGIN_GPU_SAMPLE(Postprocess)
	m_device->postRender();
	END_GPU_SAMPLE()
	END_MEASURE(postprocessMs)

	RenderDevice::Stats& stats = m_device->stats;
	stats.times.prerender = prerenderMs;
	stats.times.upload = uploadMs;
	stats.times.shadow = shadowMs;
	stats.times.reflection = reflectionMs;
	stats.times.scene = sceneMs;
	stats.times.postprocess = postprocessMs;
}

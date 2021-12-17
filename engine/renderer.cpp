#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include "components.hpp"
#include "geometry.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "image.hpp"
#include <algorithm>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/intersect.hpp>

#define USE_GRANULAR_GPU_PROFILER 0
#if defined(USE_PROFILER) && USE_GRANULAR_GPU_PROFILER
#define BEGIN_ENTITY_GPU_SAMPLE(prefix, ent) \
	if (ent.has<DebugInfo>()) BEGIN_GPU_SAMPLE_STRING((prefix + (" " + ent.get<DebugInfo>().name)).c_str()) \
	else BEGIN_GPU_SAMPLE_STRING(prefix)
#define END_ENTITY_GPU_SAMPLE() END_GPU_SAMPLE()
#else
#define BEGIN_ENTITY_GPU_SAMPLE(...)
#define END_ENTITY_GPU_SAMPLE()
#endif


static CVar<bool> cvar_shadows("r.shadows", true);
static CVar<bool> cvar_cubeShadows("r.cubeShadows", true);
static CVar<bool> cvar_reflections("r.reflections", true);
#ifndef SHIPPING_BUILD
static CVar<bool> cvar_freezeCullingFrustum("r.freezeCullingFrustum", false);
#endif

struct NaiveFrustum
{
	NaiveFrustum(vec3 position, vec3 dir, float far) {
		m_radius = far * 0.5f;
		m_center = position + normalize(dir) * m_radius;
	}

	inline bool visible(const Transform& transform, const Bounds& bounds) const {
		return visible(transform.position, bounds.radius * glm::compMax(transform.scale));
	}

	inline bool visible(vec3 position, float radius) const {
		float maxDist = radius + m_radius;
		return glm::distance2(m_center, position) < maxDist * maxDist;
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
	entities.for_each<Particles>([this](Entity, Particles& particles) {
		m_device->destroyParticles(particles);
	});
}

void RenderSystem::destroy(Entity entity)
{
	if (entity.has<Model>()) {
		Model& model = entity.get<Model>();
		for (int i = 0; i < Model::MAX_LODS && model.lods[i].geometry; ++i)
			m_device->destroyGeometry(*model.lods[i].geometry);
	}
	if (entity.has<Particles>()) {
		m_device->destroyParticles(entity.get<Particles>());
	}
}

void RenderSystem::toggleWireframe()
{
	m_device->toggleWireframe();
}

static inline bool useTransparentPass(Model& model) { return !model.materials.empty() && model.materials[0].blendFunc != Material::BLEND_NONE; }
static inline bool useTransparentPass(Particles& particles) { return particles.material.blendFunc != Material::BLEND_NONE; }

void RenderSystem::render(Entities& entities, Camera& camera, const Transform& camTransform)
{
	m_device->stats = RenderDevice::Stats();

	bool cubeShadows = settings.shadows && cvar_cubeShadows();
	if (!m_device->caps.geometryShaders || !m_device->caps.cubeFboAttachment)
	{
		cubeShadows = false;
		settings.dynamicReflections = false;
	}

	struct ReflectionProbe { vec3 pos; float priority; };
	static std::vector<ReflectionProbe> reflectionProbes;
	static std::vector<Light> lights;
	reflectionProbes.clear();
	lights.clear();

	START_MEASURE(prerenderMs)
	vec3 camPos = camTransform.position;
	quat camRot = camTransform.rotation;
	vec3 camDir = camRot * forward_axis;
	camera.updateViewMatrix(camPos, camRot);
	#if 1
	#ifndef SHIPPING_BUILD
	static Frustum frustum(camera);
	if (!cvar_freezeCullingFrustum()) {
		frustum = Frustum(camera);
	}
	#else
	Frustum frustum(camera);
	#endif
	#else
	NaiveFrustum frustum(camPos, camDir, camera.far);
	#endif

	auto calcSignedDepth = [camPos, camDir](vec3 pos) {
		float depth = glm::distance2(camPos, pos);
		if (glm::dot(camDir, pos - camPos) < 0.f)
			return -depth;
		return depth;
	};

	struct SortedDrawCall {
		SortedDrawCall(Entity e, Transform* trans, float prio): priority(prio), entity(e), transform(trans) { }
		float priority = 0.f;
		Entity entity;
		Transform* transform  = nullptr;
		Model* model = nullptr;
		Particles* particles = nullptr;
	};
	static std::vector<SortedDrawCall> sortedDrawCalls;
	sortedDrawCalls.clear();

	entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
		// Update transform
		transform.updateMatrix();
		// Update LOD
		#ifdef SHIPPING_BUILD
		model.geometry = model.getLod2(glm::distance2(camTransform.position, transform.position));
		#else
		model.geometry = settings.forceLod >= 0 ? model.lods[settings.forceLod].geometry
			: model.getLod2(glm::distance2(camPos, transform.position));
		#endif
		if (useTransparentPass(model) && !model.materials.empty() && model.geometry && frustum.visible(transform, model.bounds)) {
			sortedDrawCalls.emplace_back(e, &transform, calcSignedDepth(transform.position));
			sortedDrawCalls.back().model = &model;
		}
	});
	entities.for_each<Particles, Transform>([&](Entity e, Particles& particles, Transform& transform) {
		transform.updateMatrix();
		if (useTransparentPass(particles) && particles.count && frustum.visible(transform, particles.bounds)) {
			sortedDrawCalls.emplace_back(e, &transform, calcSignedDepth(transform.position));
			sortedDrawCalls.back().particles = &particles;
		}
	});
	std::sort(sortedDrawCalls.begin(), sortedDrawCalls.end(), [](const SortedDrawCall& a, const SortedDrawCall& b) {
		return a.priority > b.priority; // Depth sorting works the other way around
	});

	if (settings.dynamicReflections && cvar_reflections()) {
		entities.for_each<Model, Transform>([&](Entity, Model& model, Transform& transform) {
			// Figure out candidates for reflection location
			if (frustum.visible(transform, model.bounds)) {
				float reflectivity = 0.f;
				for (auto& mat : model.materials)
					if (mat.reflectivity > reflectivity)
						reflectivity = mat.reflectivity;
				if (reflectivity > 0.01f) {
					vec3 boundsMin = vec3(transform.matrix * vec4(model.bounds.min, 1.0));
					vec3 boundsMax = vec3(transform.matrix * vec4(model.bounds.max, 1.0));
					vec3 pos = glm::clamp(camPos, boundsMin, boundsMax);
					float priority = glm::distance2(camPos, pos);
					//float angleCos = glm::dot(camDir, normalize(pos - camPos));
					//priority *= (angleCos > 0.f) ? angleCos : 1.f;
					priority *= 1.1f - reflectivity;
					reflectionProbes.push_back({ pos, priority });
				}
			}
		});
		std::sort(reflectionProbes.begin(), reflectionProbes.end(), [](const ReflectionProbe& a, const ReflectionProbe& b) {
			return a.priority < b.priority;
		});
		// TODO: Prune ones that are too close to picked ones
		if (reflectionProbes.size() > MAX_REFLECTIONS)
			reflectionProbes.resize(MAX_REFLECTIONS);
	}

	// Inject sun light source
	// TODO: Just pick an existing directional light if exists?
	if (m_env.sunColor.r > 0.0 || m_env.sunColor.g > 0.0 || m_env.sunColor.b > 0.0) {
		Light sun;
		sun.type = Light::DIRECTIONAL_LIGHT;
		sun.color = m_env.sunColor;
		sun.position = camPos + normalize(m_env.sunPosition) * 10.f;
		sun.direction = normalize(camPos - sun.position);
		sun.shadowDistance = 50.f;
		sun.priority = -10000; // Should always be first
		lights.emplace_back(sun);
	}

	// Update and prioritize lights
	vec3 lightPrioTarget = camPos + camRot * (forward_axis * 2.0f);
	entities.for_each<Light, Transform>([&](Entity, Light& light, Transform& transform) {
		light.position = transform.position;
		light.shadowIndex = -1;
		if (light.type == Light::POINT_LIGHT) {
			if (!frustum.visible(light.position, light.distance))
				return;
		}
		else if (light.type == Light::SPOT_LIGHT) {
			if (!frustum.visible(light.position, light.distance)) // TODO: Spot light could use better culling...
				return;
			light.direction = transform.forward();
		}
		if (light.type == Light::DIRECTIONAL_LIGHT) {
			light.direction = transform.forward();
			light.priority = -glm::dot(light.color, vec3(1)); // Dir lights prioritized before anything else, based on intensity
		} else {
			// TODO: Better prioritizing
			light.priority = glm::distance2(lightPrioTarget, light.position);
		}
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
	entities.for_each<Particles>([&](Entity, Particles& particles) {
		// Upload particle buffers
		if (particles.renderId < 0) {
			m_device->uploadParticleBuffers(particles);
			++uploadCount;
		}
		// Upload particle materials
		auto& mat = particles.material;
		if (mat.shaderId[0] < 0 || (mat.flags & Material::DIRTY_MAPS)) {
			m_device->uploadMaterial(mat);
			++uploadCount;
		}
	});
	END_GPU_SAMPLE()
	END_MEASURE(uploadMs)
	if (uploadCount > 0)
		logDebug("Async GPU upload operations count %d in %.1fms (cpu)", uploadCount, uploadMs);

	// Compute / particle sim pass
	START_MEASURE(computeMs)
	BEGIN_GPU_SAMPLE(ComputePass)
	m_device->setupRenderPass(camera, lights, TECH_COMPUTE);
	entities.for_each<Particles, Transform>([&](Entity e, Particles& particles, Transform& transform) {
		if (!particles.computeId || particles.count == 0)
			return;
		// TODO: Culling
		BEGIN_ENTITY_GPU_SAMPLE("Compute", e)
		m_device->computeParticles(particles, transform);
		END_ENTITY_GPU_SAMPLE()
	});
	END_GPU_SAMPLE()
	END_MEASURE(computeMs)

	START_MEASURE(shadowMs)
	BEGIN_GPU_SAMPLE(ShadowPass)
	int shadowIndex = 0;
	if (settings.shadows && cvar_shadows()) {
		// Directional and spot light shadows
		uint usedShadowMaps = 0;
		for (uint i = 0; i < lights.size() && usedShadowMaps < MAX_SHADOW_MAPS; ++i) {
			Light& light = lights[i];
			if (light.type == Light::POINT_LIGHT || light.shadowDistance == 0.f)
				continue;
			BEGIN_GPU_SAMPLE(ShadowMap)
			light.shadowIndex = shadowIndex;
			m_device->setupShadowPass(light, shadowIndex);
			// TODO: Switch to using proper Frustum
			NaiveFrustum shadowFrustum(light.position, light.direction, light.shadowDistance >= 0.f ? light.shadowDistance : light.distance);
			entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
				if (!model.materials.empty() && model.geometry && shadowFrustum.visible(transform, model.bounds)) {
					BEGIN_ENTITY_GPU_SAMPLE("Shadow", e)
					m_device->renderShadow(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr);
					END_ENTITY_GPU_SAMPLE()
				}
			});
			END_GPU_SAMPLE()
			++usedShadowMaps;
			++shadowIndex;
		}

		// Point light shadows
		if (cubeShadows) { // All conditions applied to this bool already
			shadowIndex = MAX_SHADOW_MAPS; // Start from first shadow cubemap
			uint usedCubeShadows = 0;
			for (uint i = 0; i < lights.size() && usedCubeShadows < MAX_SHADOW_CUBES; ++i) {
				Light& light = lights[i];
				if (light.type != Light::POINT_LIGHT || light.shadowDistance == 0.f)
					continue;
				BEGIN_GPU_SAMPLE(ShadowCube)
				light.shadowIndex = shadowIndex;
				m_device->setupShadowPass(light, shadowIndex);
				entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
					if (model.materials.empty() || !model.geometry)
						return;
					float maxDist = model.bounds.radius * glm::compMax(transform.scale) + light.distance;
					if (glm::distance2(light.position, transform.position) < maxDist * maxDist) {
						BEGIN_ENTITY_GPU_SAMPLE("Cube shadow", e)
						m_device->renderShadow(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr);
						END_ENTITY_GPU_SAMPLE()
					}
				});
				END_GPU_SAMPLE()
				++usedCubeShadows;
				++shadowIndex;
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
		for (int i = 0; i < reflectionProbes.size(); ++i) { // Already resized to MAX_REFLECTIONS
			BEGIN_GPU_SAMPLE(ReflectionProbe)
			vec3 reflCamPos = reflectionProbes[i].pos;
			reflCam.updateViewMatrix(reflCamPos);
			m_device->setupRenderPass(reflCam, lights, TECH_REFLECTION, i);
			entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
				if (model.materials.empty() || !model.geometry)
					return;
				float maxDist = model.bounds.radius * glm::compMax(transform.scale) + reflCam.far;
				if (glm::distance2(reflCamPos, transform.position) >= maxDist * maxDist)
					return;
				BEGIN_ENTITY_GPU_SAMPLE("Reflection", e)
				m_device->render(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr);
				END_ENTITY_GPU_SAMPLE()
			});
			BEGIN_GPU_SAMPLE(ReflectionSkybox)
			m_device->renderSkybox();
			END_GPU_SAMPLE()
			END_GPU_SAMPLE()
		}
	}
	END_GPU_SAMPLE()
	END_MEASURE(reflectionMs)

	// Scene opaque color pass
	START_MEASURE(opaqueMs)
	BEGIN_GPU_SAMPLE(OpaqueGeometry)
	m_device->setupRenderPass(camera, lights, TECH_COLOR);
	entities.for_each<Model, Transform>([&](Entity e, Model& model, Transform& transform) {
		if (!model.materials.empty() && model.geometry && frustum.visible(transform, model.bounds) && !useTransparentPass(model)) {
			// Pick best reflection map
			int reflectionIndex = 0;
			if (reflectionProbes.size() > 1) {
				float bestDist = 1e6;
				for (int i = 0; i < reflectionProbes.size(); ++i) {
					float dist = glm::distance2(transform.position, reflectionProbes[i].pos);
					if (dist < bestDist) {
						bestDist = dist;
						reflectionIndex = i;
					}
				}
			}
			BEGIN_ENTITY_GPU_SAMPLE("Render", e)
			m_device->render(model, transform, e.has<BoneAnimation>() ? &e.get<BoneAnimation>() : nullptr, reflectionIndex);
			END_ENTITY_GPU_SAMPLE()
		}
	});
	END_GPU_SAMPLE()
	//BEGIN_GPU_SAMPLE(ShaderStorageBarrier)
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//END_GPU_SAMPLE()
	BEGIN_GPU_SAMPLE(OpaqueParticles)
	entities.for_each<Particles, Transform>([&](Entity e, Particles& particles, Transform& transform) {
		if (particles.count == 0)
			return;
		if (frustum.visible(transform, particles.bounds) && !useTransparentPass(particles)) {
			BEGIN_ENTITY_GPU_SAMPLE("RenderParticles", e)
			m_device->renderParticles(particles, transform);
			END_ENTITY_GPU_SAMPLE()
		}
	});
	END_GPU_SAMPLE()
	END_MEASURE(opaqueMs)

	// Skybox before transparent pass
	BEGIN_GPU_SAMPLE(Skybox)
	m_device->renderSkybox();
	END_GPU_SAMPLE()

	// Transparent render pass
	START_MEASURE(transparentMs)
	BEGIN_GPU_SAMPLE(TransparentPass)
	m_device->beginTransparency();
	for (const SortedDrawCall& cmd : sortedDrawCalls) {
		// Pre culled and checked, can just render away!
		if (cmd.model) {
			BEGIN_ENTITY_GPU_SAMPLE("Render", cmd.entity)
			m_device->render(*cmd.model, *cmd.transform, cmd.entity.has<BoneAnimation>() ? &cmd.entity.get<BoneAnimation>() : nullptr);
			END_ENTITY_GPU_SAMPLE()
		} else if (cmd.particles) {
			BEGIN_ENTITY_GPU_SAMPLE("RenderParticles", cmd.entity)
			m_device->renderParticles(*cmd.particles, *cmd.transform);
			END_ENTITY_GPU_SAMPLE()
		}
	};
	m_device->endTransparency();
	END_GPU_SAMPLE()
	END_MEASURE(transparentMs)

	START_MEASURE(postprocessMs)
	BEGIN_GPU_SAMPLE(Postprocess)
	m_device->postRender();
	END_GPU_SAMPLE()
	END_MEASURE(postprocessMs)

	RenderDevice::Stats& stats = m_device->stats;
	stats.times.prerender = prerenderMs;
	stats.times.upload = uploadMs;
	stats.times.compute = computeMs;
	stats.times.shadow = shadowMs;
	stats.times.reflection = reflectionMs;
	stats.times.opaque = opaqueMs;
	stats.times.transparent = transparentMs;
	stats.times.postprocess = postprocessMs;
}

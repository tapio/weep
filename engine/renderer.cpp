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



Renderer::Renderer(Resources& resources)
{
	m_device.reset(new RenderDevice(resources));

	// TODO: Move to Environment class?
	std::string err;
	Json def = Json::parse(resources.getText("environment.json", Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read environment: %s", err.c_str());

	ASSERT(def.is_object());
	if (def["skybox"].is_string()) {
		const string& skyboxDir = def["skybox"].string_value();
		m_env.skybox[0] = resources.getImage(skyboxDir + "px.jpg");
		m_env.skybox[1] = resources.getImage(skyboxDir + "nx.jpg");
		m_env.skybox[2] = resources.getImage(skyboxDir + "py.jpg");
		m_env.skybox[3] = resources.getImage(skyboxDir + "ny.jpg");
		m_env.skybox[4] = resources.getImage(skyboxDir + "pz.jpg");
		m_env.skybox[5] = resources.getImage(skyboxDir + "nz.jpg");
	}
	if (def["exposure"].is_number())
		m_env.exposure = def["exposure"].number_value();
	if (!def["ambient"].is_null())
		m_env.ambient = colorToVec3(def["ambient"]);
	if (!def["sunDirection"].is_null())
		m_env.sunDirection = toVec3(def["sunDirection"]);
	if (!def["sunColor"].is_null())
		m_env.sunColor = colorToVec3(def["sunColor"]);

	m_device->setEnvironment(&m_env);
}

Renderer::~Renderer()
{
	//reset();
}

void Renderer::reset(Scene& scene)
{
	logDebug("Reseting renderer");
	for (auto& model : scene.getChildren()) {
		m_device->destroyModel(model);
	}
	m_device->loadShaders();
}

void Renderer::dumpStats() const
{
	RenderDevice::Stats& stats = m_device->stats;
	logDebug("RenderDevice frame stats:\n"
		"\ttriangles:  %d\n"
		"\tprograms:   %d\n"
		"\tdraw calls: %d",
		stats.triangles, stats.programs, stats.drawCalls);
}

void Renderer::toggleWireframe()
{
	m_device->toggleWireframe();
}

void Renderer::render(Scene& scene, Camera& camera)
{
	camera.updateViewMatrix();
	Frustum frustum(camera);
	m_device->preRender(camera, scene.getLights());
	for (auto& model : scene.getChildren()) {
		if (model.material && frustum.visible(model))
			m_device->render(model);
	}
	m_device->postRender();
}

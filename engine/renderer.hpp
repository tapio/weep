#pragma once
#include "common.hpp"
#include "environment.hpp"
#include "camera.hpp"
#include <ecs/ecs.hpp>

class RenderDevice;
class Resources;
struct Transform;
struct Model;

class RenderSystem : public ecs::System
{
public:
	RenderSystem(Resources& resources);
	~RenderSystem();

	void render(ecs::Entities& entities, Camera& camera, const Transform& camTransform);
	void reset(ecs::Entities& entities);
	void destroy(ecs::Entity entity) override;

	Camera getShadowCamera(const Camera& mainCamera, const Light& light) const;

	Environment& env() { return m_env; }

	void toggleWireframe();

	RenderDevice& device() { return *m_device; }

	struct Settings {
		bool shadows = true;
		bool dynamicReflections = true;
		int forceLod = -1;
	} settings;

private:
	std::unique_ptr<RenderDevice> m_device;
	std::vector<Model*> m_models;
	Environment m_env;
};

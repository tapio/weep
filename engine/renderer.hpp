#pragma once
#include "common.hpp"
#include "environment.hpp"

class RenderDevice;
class Resources;
struct Camera;
struct Transform;
struct Model;

class RenderSystem : public System
{
public:
	RenderSystem(Resources& resources);
	~RenderSystem();

	void render(Entities& entities, Camera& camera, const Transform& camTransform);
	void reset(Entities& entities);

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

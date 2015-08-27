#pragma once
#include "common.hpp"

class RenderDevice;
class Scene;
class Resources;
struct Camera;
struct Model;

class Renderer
{
public:
	Renderer(Resources& resources);
	~Renderer();

	void render(Scene& scene, Camera& camera);
	void reset(Scene& scene);

	void dumpStats() const;
	void toggleWireframe();

private:
	std::unique_ptr<RenderDevice> m_device;
	std::vector<Model*> m_models;
};

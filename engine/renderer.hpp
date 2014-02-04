#pragma once
#include "common.hpp"

class RenderDevice;
class Scene;
struct Camera;
struct Model;

class Renderer
{
public:
	Renderer();
	~Renderer();

	void render(Scene& scene, Camera& camera);
	void reset();

private:
	RenderDevice* m_device = nullptr;
	std::vector<Model*> m_models;
};

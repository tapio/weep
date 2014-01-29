#pragma once
#include "common.hpp"

struct Model;
struct Geometry;
struct Material;

class RenderDevice
{
public:
	RenderDevice();
	~RenderDevice();

	bool uploadModel(Model& model);
	void destroyModel(Model& model);

	bool uploadGeometry(Geometry& geometry);
	bool uploadMaterial(Material& material);

	void preRender();
	void render(Model& model);
	void postRender();

private:
	uint program = 0;
	std::vector<uint> shaders;
};

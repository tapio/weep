#pragma once
#include "common.hpp"
#include "uniforms.hpp"

struct Model;
struct Geometry;
struct Material;
struct Camera;

class RenderDevice
{
public:
	RenderDevice();
	~RenderDevice();

	bool uploadModel(Model& model);
	void destroyModel(Model& model);

	bool uploadGeometry(Geometry& geometry);
	bool uploadMaterial(Material& material);

	void preRender(const Camera& camera);
	void render(Model& model);
	void postRender();

private:
	uint m_program = 0;
	UBO<UniformCommonBlock> m_commonBlock;
	UBO<UniformColorBlock> m_colorBlock;
	std::vector<uint> m_shaders;
};

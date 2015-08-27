#pragma once
#include "common.hpp"
#include "uniforms.hpp"
#include "shader.hpp"
#include "light.hpp"

struct Model;
struct Geometry;
struct Material;
struct Camera;

class RenderDevice
{
public:
	RenderDevice();
	~RenderDevice();

	void loadShaders();
	bool uploadGeometry(Geometry& geometry);
	bool uploadMaterial(Material& material);

	void preRender(const Camera& camera, const std::vector<Light>& lights);
	void render(Model& model);
	void postRender();

	void destroyModel(Model& model);

	void toggleWireframe();

	struct Caps {
		float maxAnisotropy;
	} caps;

	struct Stats
	{
		uint drawCalls = 0;
		uint programs = 0;
		uint triangles = 0;
	} stats;

private:
	struct GPUModel
	{
		uint vao = 0;
		uint vbo = 0;
		uint ebo = 0;
	};

	uint m_program = 0;
	UBO<UniformCommonBlock, 1> m_commonBlock;
	UBO<UniformColorBlock, 1> m_colorBlock;
	UBO<UniformLightBlock, MAX_LIGHTS> m_lightBlock;
	std::vector<ShaderProgram> m_shaders;
	std::map<string, int> m_shaderNames;
	std::vector<GPUModel> m_models;
};

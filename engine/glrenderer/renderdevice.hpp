#pragma once
#include "common.hpp"
#include "uniforms.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "material.hpp"

class Resources;
struct Model;
struct Geometry;
struct Camera;
struct Environment;

class RenderDevice
{
public:
	RenderDevice(Resources& resources);
	~RenderDevice();

	void setEnvironment(Environment* env) { m_env = env; }
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

	struct FBO {
		uint fbo;
		uint colorBuffer;
		uint depthBuffer;
	} m_fbo;

	void renderFullscreenQuad();
	void renderSkybox();

	GPUModel m_fullscreenQuad;
	GPUModel m_skyboxCube;
	Material m_skyboxMat;

	uint m_program = 0;
	UBO<UniformCommonBlock> m_commonBlock;
	UBO<UniformObjectBlock> m_objectBlock;
	UBO<UniformColorBlock> m_colorBlock;
	UBO<UniformLightBlock> m_lightBlock;
	std::vector<ShaderProgram> m_shaders;
	std::map<string, int> m_shaderNames;
	std::vector<GPUModel> m_models;
	Environment* m_env;
	Resources& m_resources;
};

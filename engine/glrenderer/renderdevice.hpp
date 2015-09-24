#pragma once
#include "common.hpp"
#include "uniforms.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "fbo.hpp"

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

	void setEnvironment(Environment* env);
	void loadShaders();
	bool uploadGeometry(Geometry& geometry);
	bool uploadMaterial(Material& material);
	void destroyGeometry(Geometry& geometry);

	void preRender(const Camera& camera, const std::vector<Light>& lights);
	void render(Model& model);
	void postRender();

	void toggleWireframe();

	struct Caps {
		float maxAnisotropy;
	} caps;

	struct Stats
	{
		uint drawCalls = 0;
		uint programs = 0;
		uint triangles = 0;
		uint lights = 0;
	} stats;

private:
	struct GPUModel
	{
		uint vao = 0;
		uint vbo = 0;
		uint ebo = 0;
	};


	FBO m_msaaFbo;
	FBO m_fbo;
	FBO m_pingPongFbo[2];
	FBO m_shadowFbo;

	void renderFullscreenQuad();
	void renderSkybox();

	GPUModel m_fullscreenQuad;
	GPUModel m_skyboxCube;
	Material m_skyboxMat;

	uint m_program = 0;
	bool m_wireframe = false;
	UBO<UniformCommonBlock> m_commonBlock;
	UBO<UniformObjectBlock> m_objectBlock;
	UBO<UniformMaterialBlock> m_materialBlock;
	UBO<UniformLightBlock> m_lightBlock;
	std::vector<ShaderProgram> m_shaders;
	std::map<string, int> m_shaderNames;
	std::map<void*, Texture> m_textures;
	std::vector<GPUModel> m_models;
	Environment* m_env;
	Resources& m_resources;
};

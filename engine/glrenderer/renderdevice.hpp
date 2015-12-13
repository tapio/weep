#pragma once
#include "common.hpp"
#include "uniforms.hpp"
#include "shader.hpp"
#include "components.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "fbo.hpp"

class Resources;
struct Model;
struct Transform;
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

	void setupShadowPass(const Camera& camera);
	void renderShadow(Model& model, Transform& transform);

	void preRender(const Camera& camera, const std::vector<Light>& lights);
	void render(Model& model, Transform& transform);
	void postRender();

	void resizeRenderTargets();
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
	struct GPUGeometry
	{
		uint vao = 0;
		uint vbo = 0;
		uint ebo = 0;
	};
	void destroyGeometry(GPUGeometry& geometry);

	FBO m_msaaFbo;
	FBO m_fbo;
	FBO m_pingPongFbo[2];
	FBO m_shadowFbo[MAX_SHADOWS];

	void renderFullscreenQuad();
	void renderSkybox();

	GPUGeometry m_fullscreenQuad;
	GPUGeometry m_skyboxCube;
	Material m_skyboxMat;
	Texture m_placeholderTex;

	mat4 m_shadowProj[MAX_SHADOWS];
	mat4 m_shadowView[MAX_SHADOWS];

	uint m_program = 0;
	bool m_wireframe = false;
	UBO<UniformCommonBlock> m_commonBlock;
	UBO<UniformObjectBlock> m_objectBlock;
	UBO<UniformMaterialBlock> m_materialBlock;
	UBO<UniformLightBlock> m_lightBlock;
	std::vector<ShaderProgram> m_shaders;
	std::map<string, int> m_shaderNames;
	std::map<void*, Texture> m_textures;
	std::vector<GPUGeometry> m_geometries;
	Environment* m_env;
	Resources& m_resources;
};

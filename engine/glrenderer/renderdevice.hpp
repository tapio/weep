#pragma once
#include "common.hpp"
#include "uniforms.hpp"
#include "shader.hpp"
#include "components.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "fbo.hpp"
#include <unordered_map>

class Resources;
struct Model;
struct Transform;
struct Geometry;
struct Camera;
struct Environment;
struct Batch;

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

	void setupShadowPass(const Light& light, uint index);
	void renderShadow(Model& model, Transform& transform, Animation* animation = nullptr);

	void setupRenderPass(const Camera& camera, const std::vector<Light>& lights, Technique tech = TECH_COLOR);
	void render(Model& model, Transform& transform, Animation* animation = nullptr);
	void renderSkybox();
	void postRender();

	void resizeRenderTargets();
	void toggleWireframe();

	void useProgram(const ShaderProgram& program);

	struct Caps {
		float maxAnisotropy;
		int maxSamples;
		int maxSamplers;
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

	int generateShader(uint tags);
	void setupCubeMatrices(mat4 proj, vec3 pos);
	void drawSetup(const Transform& transform, const Animation* animation = nullptr);
	void drawBatch(const Batch& batch, bool tessellate = false);
	void renderFullscreenQuad();

	FBO m_msaaFbo;
	FBO m_fbo;
	FBO m_pingPongFbo[2];
	FBO m_shadowFbo[MAX_SHADOWS];
	FBO m_reflectionFbo;

	GPUGeometry m_fullscreenQuad;
	GPUGeometry m_skyboxCube;
	Material m_skyboxMat;
	Texture m_placeholderTex;

	mat4 m_shadowProj[MAX_SHADOWS];
	mat4 m_shadowView[MAX_SHADOWS];

	uint m_program = 0;
	Technique m_tech = TECH_COLOR;
	bool m_wireframe = false;
	UBO<UniformCommonBlock> m_commonBlock;
	UBO<UniformObjectBlock> m_objectBlock;
	UBO<UniformMaterialBlock> m_materialBlock;
	UBO<UniformLightBlock> m_lightBlock;
	UBO<UniformCubeShadowBlock> m_cubeShadowBlock;
	UBO<UniformSkinningBlock> m_skinningBlock;
	UBO<UniformPostProcessBlock> m_postProcessBlock;
	std::vector<ShaderProgram> m_shaders;
	std::unordered_map<uint, int> m_shaderNames;
	std::unordered_map<uint, int> m_shaderTags;
	std::map<void*, Texture> m_textures;
	std::vector<GPUGeometry> m_geometries;
	Environment* m_env = nullptr;
	Resources& m_resources;
};

#pragma once
#include "common.hpp"
#include "buffers.hpp"
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

	bool uploadParticleBuffers(Particles& particles);
	void bindParticleBuffers(Particles& particles);
	void computeParticles(Particles& particles, Transform& transform);
	void renderParticles(Particles& particles, Transform& transform);
	void destroyParticles(Particles& particles);

	void setupShadowPass(const Camera& camera, const Light& light);
	void renderShadow(Model& model, Transform& transform, BoneAnimation* animation = nullptr);

	void setupRenderPass(const Camera& camera, const std::vector<Light>& lights, Technique tech = TECH_COLOR, int fboIndex = 0);
	void render(Model& model, Transform& transform, BoneAnimation* animation = nullptr, int reflectionIndex = 0);
	void renderSkybox();
	void postRender();

	void beginTransparency();
	void endTransparency();

	void resizeRenderTargets();
	void toggleWireframe();

	void useMaterial(Material& material);
	void useProgram(const ShaderProgram& program);
	void useProgram(uint nameHash);
	const ShaderProgram& getProgram(uint nameHash);

	struct Caps {
		bool geometryShaders = false;
		bool tessellationShaders = false;
		bool computeShaders = false;
		bool cubeFboAttachment = false;
		bool gles = false;
		float maxAnisotropy = 0;
		int maxSamples = 0;
		int maxSamplers = 0;
		int maxArrayTextureLayers = 0;
		int maxVaryingVectors = 0;
		int maxUniformBufferBindings = 0;
		int maxShaderStorageBufferBindings = 0;
		ivec3 maxComputeWorkGroupCount = {0, 0, 0};
		ivec3 maxComputeWorkGroupSize = {0, 0, 0};
		int maxComputeWorkGroupInvocations = 0;
	} caps;

	struct Stats
	{
		uint drawCalls = 0;
		uint programs = 0;
		uint triangles = 0;
		uint lights = 0;
		struct {
			float prerender = 0.f;
			float upload = 0.f;
			float compute = 0.f;
			float shadow = 0.f;
			float reflection = 0.f;
			float opaque = 0.f;
			float transparent = 0.f;
			float postprocess = 0.f;
		} times;
	} stats;

private:
	struct GPUGeometry
	{
		uint vao = 0;
		uint vbo = 0;
		uint ebo = 0;
	};
	void uploadBatch(const Batch& batch, GPUGeometry& outGeom);
	void destroyGeometry(GPUGeometry& geometry);

	struct ParticleBuffer {
		uint binding = 0;
		uint buffer = 0;
	};

	int generateShader(uint tags);
	void setupCubeMatrices(mat4 proj, vec3 pos);
	void drawSetup(const Transform& transform, const BoneAnimation* animation = nullptr, int reflectionIndex = 0);
	void drawBatch(const Batch& batch, bool tessellate = false);
	void renderFullscreenQuad();

	FBO m_msaaFbo = { "fbo_msaa" };
	FBO m_fbo = { "fbo_master" };
	FBO m_pingPongFbo[2] = { {"fbo_pingpong_a"}, {"fbo_pingpong_b"} };
	FBO m_shadowFbo[MAX_SHADOWS];
	FBO m_reflectionFbo[MAX_REFLECTIONS];

	GPUGeometry m_fullscreenQuad;
	GPUGeometry m_skyboxCube;
	GPUGeometry m_particleRenderBuffer;
	Material m_skyboxMat;
	Texture m_placeholderTex;

	mat4 m_shadowProj[MAX_SHADOWS];
	mat4 m_shadowView[MAX_SHADOWS];

	uint m_program = 0;
	Technique m_tech = TECH_COLOR;
	bool m_wireframe = false;
	UBO<UniformCommonBlock> m_commonBlock;
	UBO<UniformObjectBlock> m_objectBlock;
	UBO<UniformParticleBlock> m_particleBlock;
	UBO<UniformMaterialBlock> m_materialBlock;
	UBO<UniformLightBlock> m_lightBlock;
	UBO<UniformCubeMatrixBlock> m_cubeMatrixBlock;
	UBO<UniformSkinningBlock> m_skinningBlock;
	UBO<UniformPostProcessBlock> m_postProcessBlock;
	std::vector<ShaderProgram> m_shaders;
	std::unordered_map<uint, int> m_shaderNames;
	std::unordered_map<uint, int> m_shaderTags;
	std::unordered_map<void*, Texture> m_textures;
	std::vector<GPUGeometry> m_geometries;
	std::vector<std::vector<ParticleBuffer>> m_particleBuffers;
	Environment* m_env = nullptr;
	Resources& m_resources;
};

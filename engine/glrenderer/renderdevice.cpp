#include "renderdevice.hpp"
#include "glutil.hpp"
#include "components.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "resources.hpp"
#include "environment.hpp"
#include "image.hpp"
#include <glm/gtc/matrix_inverse.hpp>

#define DEBUG_REFLECTION 0 // Draws dynamic cubemap to skybox

static GLenum s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_LOW;

static void debugCallback(GLenum /*source*/, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* msg, const void* /*data*/)
{
	if (id == 131185) // Filter out NVIDIA "Buffer detailed info"
		return;
	if (id == 131218) // Filter out NVIDIA "Fragment Shader is going to be recompiled"
		return;
	if (severity >= s_debugMsgSeverityLevel) {
		if (type == GL_DEBUG_TYPE_ERROR)
			logError("OpenGL: %s (%u)", msg, id);
		else if (type == GL_DEBUG_TYPE_OTHER || type == GL_DEBUG_TYPE_MARKER)
			logDebug("OpenGL: %s (%u)", msg, id);
		else logWarning("OpenGL: %s (%u)", msg, id);
	}
}

static const mat4 s_shadowBiasMatrix(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f
);

static const string floatPrecisionString = "precision highp float;\n";

enum ShaderFeature {
	USE_FOG = 1 << 0,
	USE_DIFFUSE = 1 << 1,
	USE_SPECULAR = 1 << 2,
	USE_DIFFUSE_MAP = 1 << 3,
	USE_SPECULAR_MAP = 1 << 4,
	USE_NORMAL_MAP = 1 << 5,
	USE_EMISSION_MAP = 1 << 6,
	USE_PARALLAX_MAP = 1 << 7,
	USE_AO_MAP = 1 << 8,
	USE_ENV_MAP = 1 << 9,
	USE_REFLECTION_MAP = 1 << 10,
	USE_SHADOW_MAP = 1 << 11,
	USE_ALPHA_TEST = 1 << 12,
	USE_ALPHA_BLEND = 1 << 13,
	USE_TESSELLATION = 1 << 14,
	USE_ANIMATION = 1 << 15,
	USE_DEPTH = 1 << 16,
	USE_DEPTH_CUBE = 1 << 17,
	USE_CUBE_RENDER = 1 << 18,
	USE_TANGENT = 1 << 19,
	USE_VERTEX_COLOR = 1 << 20,
	NUM_SHADER_FEATURES = 21
};

RenderDevice::RenderDevice(Resources& resources)
	: m_resources(resources)
{
	logInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	logInfo("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	logInfo("OpenGL Version:  %s", glGetString(GL_VERSION));
	logInfo("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (Engine::settings["renderer"]["gldebug"].bool_value())
		s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_NOTIFICATION;
	else s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_LOW;
	glDebugMessageCallback((GLDEBUGPROC)debugCallback, NULL);

	GLint numExtensions, i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (i = 0; i < numExtensions; i++) {
		const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (strstr(ext, "geometry_shader") != 0)
			caps.geometryShaders = true;
		if (strstr(ext, "tessellation_shader") != 0)
			caps.tessellationShaders = true;
		//logDebug("%s", ext);
	}

	caps.gles = strncmp((const char*)glGetString(GL_VERSION), "OpenGL ES", strlen("OpenGL ES")) == 0;
	if (caps.gles) { // TODO: Should not force disable, but shader extensions / #version require fixing
		caps.tessellationShaders = false;
	}
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &caps.maxAnisotropy);
	glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &caps.maxSamples);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &caps.maxSamplers);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &caps.maxArrayTextureLayers);
	//logDebug("%.1f %d %d %d", caps.maxAnisotropy, caps.maxSamples, caps.maxSamplers, caps.maxArrayTextureLayers);
	if (!caps.geometryShaders)
		logDebug("No geometry shader support.");
	if (!caps.tessellationShaders)
		logDebug("No tessellation shader support.");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, Engine::width(), Engine::height());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	loadShaders();

	GLfloat quadVertices[] = {
		// Positions        // Texture Coords
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// Setup plane VAO
	glGenVertexArrays(1, &m_fullscreenQuad.vao);
	glGenBuffers(1, &m_fullscreenQuad.vbo);
	glBindVertexArray(m_fullscreenQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuad.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(ATTR_POSITION);
	glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(ATTR_TEXCOORD);
	glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glDisableVertexAttribArray(ATTR_NORMAL);

	// Create placeholder texture
	Image temp;
	temp.width = 1;
	temp.height = 1;
	temp.channels = 3;
	temp.data = {64, 64, 64};
	m_placeholderTex.create();
	m_placeholderTex.upload(temp);

	resizeRenderTargets();

	m_commonBlock.create();
	m_objectBlock.create();
	m_materialBlock.create();
	m_lightBlock.create();
	m_cubeMatrixBlock.create();
	m_skinningBlock.create();
	m_postProcessBlock.create();
}

void RenderDevice::resizeRenderTargets()
{
	if (m_msaaFbo.valid())
		m_msaaFbo.destroy();
	if (m_fbo.valid())
		m_fbo.destroy();
	for (uint i = 0; i < countof(m_pingPongFbo); ++i)
		if (m_pingPongFbo[i].valid())
			m_pingPongFbo[i].destroy();
	for (uint i = 0; i < countof(m_shadowFbo); ++i)
		if (m_shadowFbo[i].valid())
			m_shadowFbo[i].destroy();
	if (m_reflectionFbo.valid())
		m_reflectionFbo.destroy();
	// Set up floating point framebuffer to render HDR scene to
	int samples = Engine::settings["renderer"]["msaa"].number_value();
	if (caps.gles && samples > 1) {
		logWarning("MSAA is not currently supported on GLES3.");
		samples = 0;
	}
	if (samples > 1) {
		m_msaaFbo.width = Engine::width();
		m_msaaFbo.height = Engine::height();
		m_msaaFbo.numTextures = 3;
		m_msaaFbo.depthAttachment = 2;
		m_msaaFbo.samples = samples;
		m_msaaFbo.create();
	}
	m_fbo.width = Engine::width();
	m_fbo.height = Engine::height();
	m_fbo.numTextures = 3;
	m_fbo.depthAttachment = 2;
	m_fbo.create();
	for (uint i = 0; i < countof(m_pingPongFbo); ++i) {
		m_pingPongFbo[i].width = Engine::width();
		m_pingPongFbo[i].height = Engine::height();
		m_pingPongFbo[i].numTextures = 1;
		m_pingPongFbo[i].create();
	}
	for (uint i = 0; i < MAX_SHADOW_MAPS; ++i) {
		m_shadowFbo[i].width = Engine::settings["renderer"]["shadowMapSize"].number_value();
		m_shadowFbo[i].height = m_shadowFbo[i].width;
		m_shadowFbo[i].depthAttachment = 0;
		m_shadowFbo[i].cube = false;
		m_shadowFbo[i].create();
	}
	for (uint i = MAX_SHADOW_MAPS; i < MAX_SHADOWS; ++i) {
		m_shadowFbo[i].width = Engine::settings["renderer"]["shadowCubeSize"].number_value();
		m_shadowFbo[i].height = m_shadowFbo[i].width;
		m_shadowFbo[i].depthAttachment = 0;
		m_shadowFbo[i].cube = true;
		m_shadowFbo[i].create();
	}
	m_reflectionFbo.width = Engine::settings["renderer"]["reflectionCubeSize"].number_value();
	m_reflectionFbo.height = m_reflectionFbo.width;
	m_reflectionFbo.numTextures = 2;
	m_reflectionFbo.depthAttachment = 1;
	m_reflectionFbo.cube = true;
	m_reflectionFbo.create();
}

void RenderDevice::loadShaders()
{
	uint t0 = Engine::timems();
	for (auto& it : m_shaders)
		it.destroy();
	m_shaders.clear();
	m_shaderNames.clear();
	m_shaderTags.clear();
	std::string err;
	Json jsonShaders = Json::parse(m_resources.getText("shaders.json", Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read shader config: %s", err.c_str());

	ASSERT(jsonShaders.is_object());
	const Json::object& shaders = jsonShaders.object_items();
	m_shaders.reserve(shaders.size());
	m_shaderNames.reserve(shaders.size());
	for (auto& it : shaders) {
		const Json& shaderFiles = it.second["shaders"];

		if (!caps.geometryShaders && shaderFiles["geom"].is_string()) {
			logWarning("Skipping unsupported geometry shader %s", it.first.c_str());
			continue;
		}
		else if (!caps.tessellationShaders && (shaderFiles["tesc"].is_string() || shaderFiles["tese"].is_string())) {
			logWarning("Skipping unsupported tessellation shader %s", it.first.c_str());
			continue;
		}

		string file;
		m_shaders.emplace_back(it.first);
		ShaderProgram& program = m_shaders.back();

		string defineText;
		if (it.second["version"].is_string())
			defineText = "#version " + it.second["version"].string_value() + "\n";
		else defineText = "#version " + Engine::settings["renderer"]["glslversion"].string_value() + "\n";
		defineText += m_resources.getText("shaders/extensions.glsl", Resources::USE_CACHE);
		if (caps.gles)
			defineText += floatPrecisionString; // TODO: Allow precision config?

		const Json& defines = it.second["defines"];
		if (!defines.is_null()) {
			ASSERT(defines.is_array());
			for (uint i = 0; i < defines.array_items().size(); ++i) {
				const Json& def = defines[i];
				ASSERT(def.is_string());
				defineText += "#define " + def.string_value() + " 1\n";
			}
		}

		defineText += m_resources.getText("shaders/uniforms.glsl", Resources::USE_CACHE);
		defineText += "#line 1 1\n";

		file = shaderFiles["vert"].string_value();
		if (!file.empty()) program.compile(VERTEX_SHADER, m_resources.getText(file, Resources::USE_CACHE), defineText);
		file = shaderFiles["frag"].string_value();
		if (!file.empty()) program.compile(FRAGMENT_SHADER, m_resources.getText(file, Resources::USE_CACHE), defineText);
		file = shaderFiles["geom"].string_value();
		if (!file.empty()) program.compile(GEOMETRY_SHADER, m_resources.getText(file, Resources::USE_CACHE), defineText);
		file = shaderFiles["tesc"].string_value();
		if (!file.empty()) program.compile(TESS_CONTROL_SHADER, m_resources.getText(file, Resources::USE_CACHE), defineText);
		file = shaderFiles["tese"].string_value();
		if (!file.empty()) program.compile(TESS_EVALUATION_SHADER, m_resources.getText(file, Resources::USE_CACHE), defineText);
		file = shaderFiles["comp"].string_value();
		if (!file.empty()) program.compile(COMPUTE_SHADER, m_resources.getText(file, Resources::USE_CACHE), defineText);

		if (!program.link())
			continue;

		ASSERT(m_shaderNames.find(id::hash(it.first)) == m_shaderNames.end() && "Shader hash collision");
		m_shaderNames[id::hash(it.first)] = m_shaders.size() - 1;
	}
	uint t1 = Engine::timems();
	logDebug("Loaded %d shaders in %dms", m_shaders.size(), t1 - t0);
}

int RenderDevice::generateShader(uint tags)
{
	auto it = m_shaderTags.find(tags);
	if (it != m_shaderTags.end())
		return it->second;

	string name;
	if (tags & USE_DEPTH) name = "gen_depth_";
	else if (tags & USE_CUBE_RENDER) name= "gen_refl_";
	else name = "gen_color_";
	name += std::bitset<NUM_SHADER_FEATURES>(tags).to_string();
	m_shaders.emplace_back(name);
	ShaderProgram& program = m_shaders.back();

	string defineText =	"#version " + Engine::settings["renderer"]["glslversion"].string_value() + "\n";
	defineText += m_resources.getText("shaders/extensions.glsl", Resources::USE_CACHE);
	if (caps.gles)
		defineText += floatPrecisionString; // TODO: Allow precision config?

#define HANDLE_FEATURE(x) if (tags & x) defineText += "#define " #x " 1\n";
	HANDLE_FEATURE(USE_FOG)
	HANDLE_FEATURE(USE_DIFFUSE)
	HANDLE_FEATURE(USE_SPECULAR)
	HANDLE_FEATURE(USE_DIFFUSE_MAP)
	HANDLE_FEATURE(USE_SPECULAR_MAP)
	HANDLE_FEATURE(USE_NORMAL_MAP)
	HANDLE_FEATURE(USE_EMISSION_MAP)
	HANDLE_FEATURE(USE_PARALLAX_MAP)
	HANDLE_FEATURE(USE_AO_MAP)
	HANDLE_FEATURE(USE_ENV_MAP)
	HANDLE_FEATURE(USE_REFLECTION_MAP)
	HANDLE_FEATURE(USE_SHADOW_MAP)
	HANDLE_FEATURE(USE_ALPHA_TEST)
	HANDLE_FEATURE(USE_ALPHA_BLEND)
	HANDLE_FEATURE(USE_TESSELLATION)
	HANDLE_FEATURE(USE_ANIMATION)
	HANDLE_FEATURE(USE_DEPTH)
	HANDLE_FEATURE(USE_DEPTH_CUBE)
	HANDLE_FEATURE(USE_CUBE_RENDER)
	HANDLE_FEATURE(USE_TANGENT)
	HANDLE_FEATURE(USE_VERTEX_COLOR)
#undef HANDLE_FEATURE

	defineText += m_resources.getText("shaders/uniforms.glsl", Resources::USE_CACHE);
	defineText += "#line 1 1\n";

	string file = (tags & USE_DEPTH) ? "depth" : "core";
	program.compile(VERTEX_SHADER, m_resources.getText("shaders/" + file + ".vert", Resources::USE_CACHE), defineText);
	program.compile(FRAGMENT_SHADER, m_resources.getText("shaders/" + file + ".frag", Resources::USE_CACHE), defineText);
	if (tags & USE_DEPTH_CUBE)
		program.compile(GEOMETRY_SHADER, m_resources.getText("shaders/depth.geom", Resources::USE_CACHE), defineText);
	if (tags & USE_CUBE_RENDER)
		program.compile(GEOMETRY_SHADER, m_resources.getText("shaders/core.geom", Resources::USE_CACHE), defineText);
	if (tags & USE_TESSELLATION) {
		program.compile(TESS_CONTROL_SHADER, m_resources.getText("shaders/core.tesc", Resources::USE_CACHE), defineText);
		program.compile(TESS_EVALUATION_SHADER, m_resources.getText("shaders/core.tese", Resources::USE_CACHE), defineText);
	}

	if (!program.link()) {
		if ((tags & USE_DEPTH) == 0) {
			auto it = m_shaderNames.find($id(missing));
			if (it != m_shaderNames.end())
				return it->second;
		}
		return -1;
	}

	int index = m_shaders.size() - 1;
	//m_shaderNames[name] = index;
	m_shaderTags[tags] = index;
	return index;
}

RenderDevice::~RenderDevice()
{
	glDeleteBuffers(1, &m_fullscreenQuad.vbo);
	glDeleteVertexArrays(1, &m_fullscreenQuad.vao);
	m_textures.clear();
	for (auto& g : m_geometries)
		destroyGeometry(g);
}

void RenderDevice::setEnvironment(Environment* env)
{
	m_env = env;
	// TODO: Use uploadMaterial()
	m_skyboxMat.shaderName = "skybox";
	m_skyboxMat.shaderId[TECH_COLOR] = m_shaderNames[$id(skybox)];
	m_skyboxMat.shaderId[TECH_REFLECTION] = m_shaderNames[$id(skybox_refl)];
	if (!m_env->skybox[0])
		return;
	Texture& tex = m_textures[m_env->skybox[0]];
	if (!tex.valid()) {
		tex.anisotropy = caps.maxAnisotropy;
		tex.create();
		tex.uploadCube(m_env->skybox);
	}
	m_skyboxMat.tex[Material::ENV_MAP] = tex.id;
}

void RenderDevice::toggleWireframe()
{
	m_wireframe = !m_wireframe;
}

void RenderDevice::destroyGeometry(GPUGeometry& geometry)
{
	if (geometry.ebo) {
		glDeleteBuffers(1, &geometry.ebo);
		geometry.ebo = 0;
	}
	if (geometry.vbo) {
		glDeleteBuffers(1, &geometry.vbo);
		geometry.vbo = 0;
	}
	if (geometry.vao) {
		glDeleteVertexArrays(1, &geometry.vao);
		geometry.vao = 0;
	}
}

void RenderDevice::destroyGeometry(Geometry& geometry)
{
	for (auto& batch : geometry.batches) {
		if (batch.renderId == -1)
			continue;
		GPUGeometry& g = m_geometries[batch.renderId];
		destroyGeometry(g);
		batch.renderId = -1;
	}
}

bool RenderDevice::uploadGeometry(Geometry& geometry)
{
	if (geometry.batches.empty()) {
		logError("Cannot upload empty geometry");
		return false;
	}

	for (auto& batch : geometry.batches) {
		ASSERT(batch.renderId == -1);
		batch.renderId = m_geometries.size();
		m_geometries.emplace_back();
		GPUGeometry& model = m_geometries.back();

		if (!model.vao) glGenVertexArrays(1, &model.vao);
		if (!model.vbo) glGenBuffers(1, &model.vbo);
		if (!model.ebo && !batch.indices.empty()) glGenBuffers(1, &model.ebo);
		ASSERT(model.vao && model.vbo);
		glBindVertexArray(model.vao);
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
		glBufferData(GL_ARRAY_BUFFER, batch.vertexData.size(), &batch.vertexData.front(), GL_STATIC_DRAW);
		// Setup attributes
		for (int i = 0; i < ATTR_MAX; ++i) {
			Batch::Attribute& attr = batch.attributes[i];
			if (attr.components) {
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, attr.components, attr.type, attr.normalized ? GL_TRUE : GL_FALSE, batch.vertexSize, (GLvoid*)(uintptr_t)attr.offset);
			} else {
				glDisableVertexAttribArray(i);
			}
		}
		// Elements
		if (model.ebo) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch.indices.size() * sizeof(uint), &batch.indices.front(), GL_STATIC_DRAW);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;
}

bool RenderDevice::uploadMaterial(Material& material)
{
	uint tag = USE_FOG | USE_DIFFUSE;
	if (material.shininess > 0.f)
		tag |= USE_SPECULAR;
	if (material.flags & Material::TESSELLATE)
		tag |= USE_TESSELLATION;
	if (material.flags & Material::RECEIVE_SHADOW)
		tag |= USE_SHADOW_MAP;
	if (material.flags & Material::ANIMATED)
		tag |= USE_ANIMATION;
	if (material.flags & Material::ALPHA_TEST)
		tag |= USE_ALPHA_TEST;
	if (material.map[Material::DIFFUSE_MAP])
		tag |= USE_DIFFUSE_MAP | USE_DIFFUSE;
	if (material.map[Material::NORMAL_MAP])
		tag |= USE_NORMAL_MAP;
	if (material.map[Material::SPECULAR_MAP])
		tag |= USE_SPECULAR_MAP | USE_SPECULAR;
	if (material.map[Material::HEIGHT_MAP] && material.parallax > 0.f)
		tag |= USE_PARALLAX_MAP;
	if (material.map[Material::EMISSION_MAP])
		tag |= USE_EMISSION_MAP;
	if (material.map[Material::AO_MAP])
		tag |= USE_AO_MAP;
	if (material.map[Material::REFLECTION_MAP])
		tag |= USE_REFLECTION_MAP;
	if (material.reflectivity > 0.f)
		tag |= USE_ENV_MAP;

	if (!material.shaderName.empty()) {
		auto it = m_shaderNames.find(id::hash(material.shaderName));
		if (it == m_shaderNames.end()) {
			logError("Failed to find shader \"%s\"", material.shaderName.c_str());
			it = m_shaderNames.find($id(missing));
			if (it == m_shaderNames.end())
				return false;
		}
		material.shaderId[TECH_COLOR] = it->second;
		ASSERT(material.shaderId[TECH_COLOR]);
	} else {
		// Auto-generate shader
		material.shaderId[TECH_COLOR] = generateShader(tag);
		ASSERT(material.shaderId[TECH_COLOR] >= 0 && "Color shader generating failed");
	}
	// Other techs are always auto generated
	{
		// Simpler reflection shader
		if (caps.geometryShaders) {
			tag &= ~(USE_SHADOW_MAP | USE_AO_MAP | USE_REFLECTION_MAP | USE_PARALLAX_MAP | USE_ENV_MAP | USE_TESSELLATION);
			material.shaderId[TECH_REFLECTION] = generateShader(tag | USE_CUBE_RENDER);
			ASSERT(material.shaderId[TECH_REFLECTION] >= 0 && "Reflection shader generating failed");
		}

		// Depth
		tag = USE_DEPTH;
		if (material.flags & Material::ANIMATED)
			tag |= USE_ANIMATION;
		if (material.flags & Material::ALPHA_TEST)
			tag |= USE_ALPHA_TEST | USE_DIFFUSE_MAP;
		material.shaderId[TECH_DEPTH] = generateShader(tag);
		ASSERT(material.shaderId[TECH_DEPTH] >= 0 && "Depth shader generating failed");
		if (caps.geometryShaders) {
			material.shaderId[TECH_DEPTH_CUBE] = generateShader(tag | USE_DEPTH_CUBE);
			ASSERT(material.shaderId[TECH_DEPTH_CUBE] >= 0 && "Depth cube shader generating failed");
		}
	}

	bool dirty = false;
	for (uint i = 0; i < countof(material.map); ++i) {
		bool goodTex = material.tex[i] && material.tex[i] != m_placeholderTex.id;
		if (goodTex || !material.map[i])
			continue;
		if (!goodTex && material.map[i] && material.map[i]->data.empty()) {
			material.tex[i] = m_placeholderTex.id;
			dirty = true;
			continue;
		}
		Texture& tex = m_textures[material.map[i]];
		if (!tex.valid()) {
			tex.anisotropy = caps.maxAnisotropy;
			tex.create();
			tex.upload(*material.map[i]);
		}
		material.tex[i] = tex.id;
	}
	if (!dirty)
		material.flags &= ~Material::DIRTY_MAPS;
	return true;
}

void RenderDevice::useProgram(const ShaderProgram& program)
{
	uint programId = program.id;
	if (m_program != programId) {
		m_program = programId;
		glUseProgram(m_program);
		++stats.programs;
	}
}

void RenderDevice::drawSetup(const Transform& transform, const BoneAnimation* animation)
{
	m_objectBlock.uniforms.modelMatrix = transform.matrix;
	mat4 modelView = m_commonBlock.uniforms.viewMatrix * m_objectBlock.uniforms.modelMatrix;
	m_objectBlock.uniforms.modelViewMatrix = modelView;
	m_objectBlock.uniforms.modelViewProjMatrix = m_commonBlock.uniforms.projectionMatrix * modelView;
	m_objectBlock.uniforms.normalMatrix = glm::inverseTranspose(modelView);
	m_objectBlock.upload();

	if (animation && !animation->bones.empty()) {
		ASSERT(animation->bones.size() <= MAX_BONES);
		uint numBones = std::min((uint)animation->bones.size(), (uint)MAX_BONES);
		memcpy(&m_skinningBlock.uniforms.boneMatrices[0], &animation->bones[0], numBones * sizeof(mat3x4));
		m_skinningBlock.upload();
	}
}

void RenderDevice::setupCubeMatrices(mat4 proj, vec3 pos)
{
	mat4* cubeMatrices = &m_cubeMatrixBlock.uniforms.cubeMatrices[0];
	cubeMatrices[0] = proj * glm::lookAt(pos, pos + vec3(1, 0, 0), vec3(0, -1, 0));
	cubeMatrices[1] = proj * glm::lookAt(pos, pos + vec3(-1, 0, 0), vec3(0, -1, 0));
	cubeMatrices[2] = proj * glm::lookAt(pos, pos + vec3(0, 1, 0), vec3(0, 0, 1));
	cubeMatrices[3] = proj * glm::lookAt(pos, pos + vec3(0, -1, 0), vec3(0, 0, -1));
	cubeMatrices[4] = proj * glm::lookAt(pos, pos + vec3(0, 0, 1), vec3(0, -1, 0));
	cubeMatrices[5] = proj * glm::lookAt(pos, pos + vec3(0, 0, -1), vec3(0, -1, 0));
	m_cubeMatrixBlock.upload();
}

void RenderDevice::setupShadowPass(const Light& light, uint index)
{
	ASSERT(m_env);
	m_shadowFbo[index].bind();
	glViewport(0, 0, m_shadowFbo[index].width, m_shadowFbo[index].height);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	m_program = 0;
	glUseProgram(0);
	float& near = m_commonBlock.uniforms.near;
	float& far = m_commonBlock.uniforms.far;
	if (light.type == Light::POINT_LIGHT) {
		m_tech = TECH_DEPTH_CUBE;
		float aspect = (float)m_shadowFbo[index].width / (float)m_shadowFbo[index].height;
		near = 0.2f; far = light.distance;
		m_shadowProj[index] = glm::perspective(glm::radians(90.0f), aspect, near, far);
		setupCubeMatrices(m_shadowProj[index], light.position);
	} else if (light.type == Light::DIRECTIONAL_LIGHT) {
		m_tech = TECH_DEPTH;
		// TODO: Configure
		float size = 20.f;
		near = 1.f; far = 50.f;
		m_shadowProj[index] = glm::ortho(-size, size, -size, size, near, far);
		m_shadowView[index] = glm::lookAt(light.position, light.target, vec3(0, 1, 0));
	} else ASSERT(!"Unsupported light type for shadow pass");

	m_commonBlock.uniforms.projectionMatrix = m_shadowProj[index];
	m_commonBlock.uniforms.viewMatrix = m_shadowView[index];
	m_commonBlock.uniforms.cameraPosition = light.position;
	m_commonBlock.upload();
}

void RenderDevice::renderShadow(Model& model, Transform& transform, BoneAnimation* animation)
{
	drawSetup(transform, animation);

	Geometry& geom = *model.geometry;
	for (auto& batch : geom.batches) {

		ASSERT(batch.materialIndex <= model.materials.size());
		Material& mat = model.materials[batch.materialIndex];
		if (!(mat.flags & Material::CAST_SHADOW))
			continue;

		useProgram(m_shaders[mat.shaderId[m_tech]]);

		if (mat.flags & Material::ALPHA_TEST) {
			glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_MAP);
			glBindTexture(GL_TEXTURE_2D, mat.tex[Material::DIFFUSE_MAP]);
		}

		drawBatch(batch);
	}
	glBindVertexArray(0);
}

void RenderDevice::setupRenderPass(const Camera& camera, const std::vector<Light>& lights, Technique tech)
{
	ASSERT(m_env);
	FBO* fbo;
	if (tech == TECH_REFLECTION) fbo = &m_reflectionFbo;
	else if (m_msaaFbo.valid()) fbo = &m_msaaFbo;
	else fbo = &m_fbo;
	fbo->bind();
	glViewport(0, 0, fbo->width, fbo->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	m_tech = tech;
	m_program = 0;
	glUseProgram(0);
	m_commonBlock.uniforms.projectionMatrix = camera.projection;
	m_commonBlock.uniforms.viewMatrix = camera.view;
	m_commonBlock.uniforms.cameraPosition = camera.view[3];
	m_commonBlock.uniforms.globalAmbient = m_env->ambient;
	m_commonBlock.uniforms.shadowDarkness = m_env->shadowDarkness;
	m_commonBlock.uniforms.bloomThreshold = m_env->bloomThreshold;
	m_commonBlock.uniforms.skyType = m_env->skyType;
	m_commonBlock.uniforms.sunPosition = glm::normalize(m_env->sunPosition);
	m_commonBlock.uniforms.sunColor = m_env->sunColor;
	m_commonBlock.uniforms.fogColor = m_env->fogColor;
	m_commonBlock.uniforms.fogDensity = m_env->fogDensity;
	m_commonBlock.uniforms.near = camera.near;
	m_commonBlock.uniforms.far = camera.far;

	uint numLights = std::min((int)lights.size(), MAX_LIGHTS);
	m_commonBlock.uniforms.numLights = numLights;
	stats.lights = numLights;
	if (numLights) {
		for (uint i = 0; i < numLights; i++) {
			const Light& light = lights[i];
			m_lightBlock.uniforms.lights[i].color = light.color;
			m_lightBlock.uniforms.lights[i].position = light.position;
			m_lightBlock.uniforms.lights[i].direction = light.direction;
			m_lightBlock.uniforms.lights[i].params = vec4(light.distance, light.decay, 0.0f, 0.0f);
		}
	}
	m_lightBlock.upload();
	m_commonBlock.upload();

	if (tech == TECH_REFLECTION)
		setupCubeMatrices(m_commonBlock.uniforms.projectionMatrix, camera.view[3]);

	// Shadow map textures
	for (uint i = 0; i < countof(m_shadowFbo); ++i) {
		glActiveTexture(GL_TEXTURE0 + BINDING_SHADOW_MAP + i);
		glBindTexture(m_shadowFbo[i].cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, m_shadowFbo[i].tex[0]);
	}

	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void RenderDevice::render(Model& model, Transform& transform, BoneAnimation* animation)
{
	m_objectBlock.uniforms.shadowMatrix = s_shadowBiasMatrix * (m_shadowProj[0] * (m_shadowView[0] * transform.matrix));
	drawSetup(transform, animation);

	uint envTex = m_tech == TECH_COLOR ? m_reflectionFbo.tex[0] : m_skyboxMat.tex[Material::ENV_MAP];
	glActiveTexture(GL_TEXTURE0 + BINDING_ENV_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envTex);

	Geometry& geom = *model.geometry;
	for (auto& batch : geom.batches) {

		ASSERT(batch.materialIndex < model.materials.size());
		Material& mat = model.materials[batch.materialIndex];
		ASSERT(mat.shaderId[m_tech] >= 0);

		useProgram(m_shaders[mat.shaderId[m_tech]]);

		m_materialBlock.uniforms.ambient = mat.ambient;
		m_materialBlock.uniforms.diffuse = mat.diffuse;
		m_materialBlock.uniforms.specular = mat.specular;
		m_materialBlock.uniforms.shininess = mat.shininess;
		m_materialBlock.uniforms.reflectivity = mat.reflectivity;
		m_materialBlock.uniforms.parallax = mat.parallax;
		m_materialBlock.uniforms.emissive = mat.emissive;
		m_materialBlock.uniforms.uvOffset = mat.uvOffset;
		m_materialBlock.uniforms.uvRepeat = mat.uvRepeat;
		m_materialBlock.upload();

		for (uint i = 0; i < Material::ENV_MAP; ++i) {
			uint tex = mat.tex[i];
			if (!tex) continue;
			glActiveTexture(GL_TEXTURE0 + BINDING_MATERIAL_MAP_START + i);
			glBindTexture(GL_TEXTURE_2D, tex);
			//glUniform1i(i, i);
		}

		drawBatch(batch, m_tech == TECH_COLOR && (mat.flags & Material::TESSELLATE));
	}
	glBindVertexArray(0);
}


void RenderDevice::drawBatch(const Batch& batch, bool tessellate)
{
	ASSERT(batch.renderId >= 0);
	GPUGeometry& gpuData = m_geometries[batch.renderId];
	glBindVertexArray(gpuData.vao);
	uint mode = tessellate ? GL_PATCHES : GL_TRIANGLES;
	if (gpuData.ebo) {
		glDrawElements(mode, batch.indices.size(), GL_UNSIGNED_INT, 0);
		stats.triangles += batch.indices.size() / 3;
	} else {
		glDrawArrays(mode, 0, batch.numVertices);
		stats.triangles += batch.numVertices / 3;
	}
	++stats.drawCalls;
}

void RenderDevice::renderFullscreenQuad()
{
	glBindVertexArray(m_fullscreenQuad.vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	++stats.drawCalls;
	stats.triangles += 2;
	glBindVertexArray(0);
}

void RenderDevice::renderSkybox()
{
	if (m_skyboxMat.shaderId[TECH_COLOR] == -1)
		return;
	if (!m_skyboxCube.vao) {
		GLfloat skyboxVertices[] = {
			-1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,

			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f
		};
		glGenVertexArrays(1, &m_skyboxCube.vao);
		glGenBuffers(1, &m_skyboxCube.vbo);
		glBindVertexArray(m_skyboxCube.vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_skyboxCube.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(ATTR_POSITION);
		glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	}
	glDepthFunc(GL_LEQUAL);
	glUseProgram(m_shaders[m_skyboxMat.shaderId[m_tech]].id);
	if (m_tech == TECH_REFLECTION) {
		setupCubeMatrices(m_commonBlock.uniforms.projectionMatrix, vec3(0, 0, 0));
	} else {
		// Remove translation
		m_commonBlock.uniforms.viewMatrix = glm::mat4(glm::mat3(m_commonBlock.uniforms.viewMatrix));
		m_commonBlock.upload();
	}
	glBindVertexArray(m_skyboxCube.vao);
	glActiveTexture(GL_TEXTURE0 + BINDING_ENV_MAP);
	if (DEBUG_REFLECTION && m_tech == TECH_COLOR)
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_reflectionFbo.tex[0]);
	else glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxMat.tex[Material::ENV_MAP]);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	++stats.drawCalls;
	stats.triangles += 36;
}

void RenderDevice::postRender()
{
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_postProcessBlock.uniforms.tonemap = m_env->tonemap;
	m_postProcessBlock.uniforms.exposure = m_env->exposure;
	m_postProcessBlock.uniforms.saturation = m_env->saturation;
	m_postProcessBlock.uniforms.postAA = m_env->postAA;
	m_postProcessBlock.uniforms.chromaticAberration = m_env->chromaticAberration;
	m_postProcessBlock.uniforms.sepia = m_env->sepia;
	m_postProcessBlock.uniforms.vignette = m_env->vignette;
	m_postProcessBlock.uniforms.scanlines = m_env->scanlines;
	m_postProcessBlock.upload();

	// Resolve MSAA to regular FBO
	if (m_msaaFbo.valid()) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo.fbo);
		GLenum buf = GL_COLOR_ATTACHMENT0;
		glReadBuffer(buf);
		glDrawBuffers(1, &buf);
		glBlitFramebuffer(0, 0, m_msaaFbo.width, m_msaaFbo.height, 0, 0, m_fbo.width, m_fbo.height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		buf = GL_COLOR_ATTACHMENT1;
		glReadBuffer(buf);
		glDrawBuffers(1, &buf);
		glBlitFramebuffer(0, 0, m_msaaFbo.width, m_msaaFbo.height, 0, 0, m_fbo.width, m_fbo.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	uint pingpong = 0;
	//if (m_env->bloomIntensity >= 1.f && m_env->bloomThreshold > 0.f)
	{
		// Blur bloom texture
		uint amount = (uint)m_env->bloomIntensity;
		glUseProgram(m_shaders[m_shaderNames[$id(hblur)]].id);
		glActiveTexture(GL_TEXTURE20);
		for (uint i = 0; i < amount; i++)
		{
			m_pingPongFbo[pingpong].bind();
			glBindTexture(GL_TEXTURE_2D, i == 0 ? m_fbo.tex[1] : m_pingPongFbo[!pingpong].tex[0]);
			renderFullscreenQuad();
			pingpong = !pingpong;
		}
		glUseProgram(m_shaders[m_shaderNames[$id(vblur)]].id);
		for (uint i = 0; i < amount; i++)
		{
			m_pingPongFbo[pingpong].bind();
			glBindTexture(GL_TEXTURE_2D, m_pingPongFbo[!pingpong].tex[0]);
			renderFullscreenQuad();
			pingpong = !pingpong;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_shaders[m_shaderNames[$id(postfx)]].id);
	++stats.programs;
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, m_fbo.tex[0]);
	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, m_pingPongFbo[!pingpong].tex[0]);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, m_fbo.tex[2]);
	renderFullscreenQuad();

	glUseProgram(0);
}

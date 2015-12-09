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

static GLenum s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_LOW;

static void debugCallback(GLenum /*source*/, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* msg, const void* /*data*/)
{
	if (id == 131185) // Filter out NVIDIA "Buffer detailed info"
		return;
	if (severity >= s_debugMsgSeverityLevel) {
		if (type == GL_DEBUG_TYPE_ERROR)
			logError("OpenGL: %s", msg);
		else if (type == GL_DEBUG_TYPE_OTHER || type == GL_DEBUG_TYPE_MARKER)
			logDebug("OpenGL: %s", msg);
		else logWarning("OpenGL: %s", msg);
	}
}

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

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &caps.maxAnisotropy);

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

	resizeRenderTargets();

	m_commonBlock.create();
	m_objectBlock.create();
	m_materialBlock.create();
	m_lightBlock.create();
}

void RenderDevice::resizeRenderTargets()
{
	if (m_msaaFbo.valid())
		m_msaaFbo.destroy();
	if (m_fbo.valid())
		m_fbo.destroy();
	for (int i = 0; i < 2; ++i)
		if (m_pingPongFbo[i].valid())
			m_pingPongFbo[i].destroy();
	if (m_shadowFbo.valid())
		m_shadowFbo.destroy();
	// Set up floating point framebuffer to render HDR scene to
	int samples = Engine::settings["renderer"]["msaa"].number_value();
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
	for (int i = 0; i < 2; ++i) {
		m_pingPongFbo[i].width = Engine::width();
		m_pingPongFbo[i].height = Engine::height();
		m_pingPongFbo[i].numTextures = 1;
		m_pingPongFbo[i].create();
	}
	m_shadowFbo.width = Engine::settings["renderer"]["shadowMapSize"].number_value();
	m_shadowFbo.height = m_shadowFbo.width;
	m_shadowFbo.depthAttachment = 0;
	m_shadowFbo.create();
	glutil::checkGL("Post framebuffer create");
}

void RenderDevice::loadShaders()
{
	uint t0 = Engine::timems();
	m_shaders.clear();
	m_shaderNames.clear();
	std::string err;
	Json jsonShaders = Json::parse(m_resources.getText("shaders.json", Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read shader config: %s", err.c_str());

	ASSERT(jsonShaders.is_object());
	const Json::object& shaders = jsonShaders.object_items();
	for (auto& it : shaders) {
		const Json& shaderFiles = it.second["shaders"];
		string file;
		m_shaders.emplace_back(it.first);
		ShaderProgram& program = m_shaders.back();

		string defineText;
		if (it.second["version"].is_string())
			defineText = "#version " + it.second["version"].string_value() + "\n";
		else defineText =
			"#version 330 core\n"
			"#extension GL_ARB_shading_language_420pack : enable\n"
			"#extension GL_ARB_explicit_uniform_location : enable\n"
			"\n";

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

		if (!program.link())
			continue;

		m_shaderNames[it.first] = m_shaders.size() - 1;
		logDebug("Shader \"%s\" initialized as %d (shader mask: %d%d%d%d%d)",
			it.first.c_str(),
			program.id,
			program.has(VERTEX_SHADER),
			program.has(FRAGMENT_SHADER),
			program.has(GEOMETRY_SHADER),
			program.has(TESS_CONTROL_SHADER),
			program.has(TESS_EVALUATION_SHADER)
		);
	}
	uint t1 = Engine::timems();
	logDebug("Loaded %d shaders in %dms", m_shaders.size(), t1 - t0);
}

RenderDevice::~RenderDevice()
{
	glDeleteBuffers(1, &m_fullscreenQuad.vbo);
	glDeleteVertexArrays(1, &m_fullscreenQuad.vao);
	m_commonBlock.destroy();
	m_objectBlock.destroy();
	m_materialBlock.destroy();
	m_lightBlock.destroy();
	m_textures.clear();
	for (auto& g : m_geometries)
		destroyGeometry(g);
}

void RenderDevice::setEnvironment(Environment* env)
{
	m_env = env;
	// TODO: Use uploadMaterial()
	m_skyboxMat.shaderName = "skybox";
	m_skyboxMat.shaderId = m_shaders[m_shaderNames["skybox"]].id;
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
	glutil::checkGL("Pre geometry upload");
	for (auto& batch : geometry.batches) {
		batch.renderId = m_geometries.size();
		m_geometries.emplace_back(GPUGeometry());
		GPUGeometry& model = m_geometries.back();

		if (!model.vao) glGenVertexArrays(1, &model.vao);
		if (!model.vbo) glGenBuffers(1, &model.vbo);
		if (!model.ebo && !batch.indices.empty()) glGenBuffers(1, &model.ebo);
		glBindVertexArray(model.vao);
		glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
		glBufferData(GL_ARRAY_BUFFER, batch.vertexData.size(), &batch.vertexData.front(), GL_STATIC_DRAW);
		// Setup attributes
		for (int i = 0; i < ATTR_MAX; ++i) {
			Batch::Attribute& attr = batch.attributes[i];
			if (attr.components) {
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, attr.components, GL_FLOAT, GL_FALSE, batch.vertexSize, (GLvoid*)(uintptr_t)attr.offset);
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
	glutil::checkGL("Post geometry upload");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;
}

bool RenderDevice::uploadMaterial(Material& material)
{
	auto it = m_shaderNames.find(material.shaderName);
	if (it == m_shaderNames.end()) {
		logError("Failed to find shader \"%s\"", material.shaderName.c_str());
		it = m_shaderNames.find("missing");
		if (it == m_shaderNames.end())
			return false;
	}
	material.shaderId = it->second;

	bool dirty = false;
	for (uint i = 0; i < material.map.size(); ++i) {
		if (!material.map[i] || material.tex[i])
			continue;
		if (!material.tex[i] && material.map[i] && material.map[i]->data.empty()) {
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

void RenderDevice::preRender(const Camera& camera, const std::vector<Light>& lights)
{
	ASSERT(m_env);
	if (m_msaaFbo.valid()) m_msaaFbo.bind();
	else m_fbo.bind();
	glViewport(0, 0, Engine::width(), Engine::height());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	stats = Stats();
	m_program = 0;
	glUseProgram(0);
	m_commonBlock.uniforms.projectionMatrix = camera.projection;
	m_commonBlock.uniforms.viewMatrix = camera.view;
	m_commonBlock.uniforms.cameraPosition = camera.position;
	m_commonBlock.uniforms.globalAmbient = m_env->ambient;
	m_commonBlock.uniforms.exposure = m_env->exposure;
	m_commonBlock.uniforms.tonemap = m_env->tonemap;
	m_commonBlock.uniforms.bloomThreshold = m_env->bloomThreshold;
	m_commonBlock.uniforms.sunDirection = glm::normalize(m_env->sunDirection);
	m_commonBlock.uniforms.sunColor = m_env->sunColor;
	m_commonBlock.uniforms.fogColor = m_env->fogColor;
	m_commonBlock.uniforms.fogDensity = m_env->fogDensity;
	m_commonBlock.uniforms.near = camera.near;
	m_commonBlock.uniforms.far = camera.far;
	m_commonBlock.uniforms.vignette = m_env->vignette;

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
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void RenderDevice::render(Model& model, Transform& transform)
{
	Geometry& geom = *model.geometry;

	m_objectBlock.uniforms.modelMatrix = transform.matrix;
	mat4 modelView = m_commonBlock.uniforms.viewMatrix * m_objectBlock.uniforms.modelMatrix;
	m_objectBlock.uniforms.modelViewMatrix = modelView;
	m_objectBlock.uniforms.normalMatrix = glm::inverseTranspose(modelView);
	m_objectBlock.upload();

	if (m_skyboxMat.shaderId != -1) {
		uint tex = m_skyboxMat.tex[Material::ENV_MAP];
		glActiveTexture(GL_TEXTURE10 + Material::ENV_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	}

	for (auto& batch : geom.batches) {

		Material& mat = *model.materials[batch.materialIndex];
		if (mat.shaderId < 0 || (mat.flags & Material::DIRTY_MAPS))
			uploadMaterial(mat); // TODO: Should not be here!

		uint programId = m_shaders[mat.shaderId].id;
		if (m_program != programId) {
			m_program = programId;
			glUseProgram(m_program);
			++stats.programs;
		}

		m_materialBlock.uniforms.ambient = mat.ambient;
		m_materialBlock.uniforms.diffuse = mat.diffuse;
		m_materialBlock.uniforms.specular = mat.specular;
		m_materialBlock.uniforms.shininess = mat.shininess;
		m_materialBlock.uniforms.emissive = mat.emissive;
		m_materialBlock.uniforms.uvOffset = mat.uvOffset;
		m_materialBlock.uniforms.uvRepeat = mat.uvRepeat;
		m_materialBlock.upload();

		for (uint i = 0; i < Material::ENV_MAP; ++i) {
			uint tex = mat.tex[i];
			if (!tex) continue;
			glActiveTexture(GL_TEXTURE10 + i);
			glBindTexture(GL_TEXTURE_2D, tex);
			//glUniform1i(i, i);
		}

		if (batch.renderId == -1)
			uploadGeometry(geom); // TODO: Should not be here!

		GPUGeometry& gpuData = m_geometries[batch.renderId];
		glBindVertexArray(gpuData.vao);
		uint mode = (mat.flags & Material::TESSELLATE) ? GL_PATCHES : GL_TRIANGLES;
		if (gpuData.ebo) {
			glDrawElements(mode, batch.indices.size(), GL_UNSIGNED_INT, 0);
			stats.triangles += batch.indices.size() / 3;
		} else {
			glDrawArrays(mode, 0, batch.numVertices);
			stats.triangles += batch.numVertices / 3;
		}
		++stats.drawCalls;
	}
	glBindVertexArray(0);
	glutil::checkGL("Post draw");
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
	if (!m_env->skybox[0] || m_skyboxMat.shaderId == -1)
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
	glUseProgram(m_skyboxMat.shaderId);
	// Remove translation
	m_commonBlock.uniforms.viewMatrix = glm::mat4(glm::mat3(m_commonBlock.uniforms.viewMatrix));
	m_commonBlock.upload();
	glBindVertexArray(m_skyboxCube.vao);
	glActiveTexture(GL_TEXTURE10 + Material::ENV_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxMat.tex[Material::ENV_MAP]);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	++stats.drawCalls;
	stats.triangles += 36;
}

void RenderDevice::postRender()
{
	renderSkybox();
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Resolve MSAA to regular FBO
	if (m_msaaFbo.valid()) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo.fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, m_msaaFbo.width, m_msaaFbo.height, 0, 0, m_fbo.width, m_fbo.height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, m_msaaFbo.width, m_msaaFbo.height, 0, 0, m_fbo.width, m_fbo.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	uint pingpong = 0;
	//if (m_env->bloomIntensity >= 1.f && m_env->bloomThreshold > 0.f)
	{
		// Blur bloom texture
		uint amount = (uint)m_env->bloomIntensity;
		glUseProgram(m_shaders[m_shaderNames["hblur"]].id);
		glActiveTexture(GL_TEXTURE20);
		for (uint i = 0; i < amount; i++)
		{
			m_pingPongFbo[pingpong].bind();
			glBindTexture(GL_TEXTURE_2D, i == 0 ? m_fbo.tex[1] : m_pingPongFbo[!pingpong].tex[0]);
			renderFullscreenQuad();
			pingpong = !pingpong;
		}
		glUseProgram(m_shaders[m_shaderNames["vblur"]].id);
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
	glUseProgram(m_shaders[m_shaderNames["postfx"]].id);
	++stats.programs;
	glActiveTexture(GL_TEXTURE20);
	glBindTexture(GL_TEXTURE_2D, m_fbo.tex[0]);
	glActiveTexture(GL_TEXTURE21);
	glBindTexture(GL_TEXTURE_2D, m_pingPongFbo[!pingpong].tex[0]);
	glActiveTexture(GL_TEXTURE22);
	glBindTexture(GL_TEXTURE_2D, m_fbo.tex[2]);
	renderFullscreenQuad();

	glUseProgram(0);
	glutil::checkGL("Post render");
}

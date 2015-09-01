#include "renderdevice.hpp"
#include "glutil.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "light.hpp"
#include "resources.hpp"
#include "environment.hpp"

static GLenum s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_LOW;

static void debugCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum severity, GLsizei /*length*/, const GLchar* msg, const void* /*data*/)
{
	if (severity >= s_debugMsgSeverityLevel)
		logDebug("OpenGL: %s", msg);
}

RenderDevice::RenderDevice(Resources& resources)
	: m_resources(resources)
{
	logInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	logInfo("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	logInfo("OpenGL Version:  %s", glGetString(GL_VERSION));
	logInfo("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	//if (Engine::settings["renderer"]["gldebug"].bool_value())
	//	s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_NOTIFICATION;
	//else s_debugMsgSeverityLevel = GL_DEBUG_SEVERITY_LOW;
	glDebugMessageCallback(debugCallback, NULL);

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

	// Set up floating point framebuffer to render scene to
	glGenFramebuffers(1, &m_fbo.fbo);
	// Create floating point color buffer for HDR rendering
	glGenTextures(1, &m_fbo.colorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_fbo.colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Engine::width(), Engine::height(), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Create depth buffer (renderbuffer)
	glGenRenderbuffers(1, &m_fbo.depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_fbo.depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Engine::width(), Engine::height());
	// Attach buffers
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo.colorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fbo.depthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		logError("Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_commonBlock.create();
	m_objectBlock.create();
	m_colorBlock.create();
	m_lightBlock.create();
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
	glDeleteRenderbuffers(1, &m_fbo.depthBuffer);
	glDeleteTextures(1, &m_fbo.colorBuffer);
	glDeleteFramebuffers(1, &m_fbo.fbo);

	glDeleteBuffers(1, &m_fullscreenQuad.vbo);
	glDeleteVertexArrays(1, &m_fullscreenQuad.vao);
	m_commonBlock.destroy();
	m_objectBlock.destroy();
	m_colorBlock.destroy();
	m_lightBlock.destroy();
}

void RenderDevice::destroyModel(Model& model)
{
	if (model.geometry->renderId == -1)
		return;
	GPUModel& m = m_models[model.geometry->renderId];
	if (m.ebo) {
		glDeleteBuffers(1, &m.ebo);
		m.ebo = 0;
	}
	if (m.vbo) {
		glDeleteBuffers(1, &m.vbo);
		m.vbo = 0;
	}
	if (m.vao) {
		glDeleteVertexArrays(1, &m.vao);
		m.vao = 0;
	}
	model.geometry->renderId = -1;
}

void RenderDevice::toggleWireframe()
{
	GLint mode;
	glGetIntegerv(GL_POLYGON_MODE, &mode);
	glPolygonMode(GL_FRONT_AND_BACK, mode == GL_LINE ? GL_FILL : GL_LINE);
}

bool RenderDevice::uploadGeometry(Geometry& geometry)
{
	if (geometry.vertexData.empty()) {
		logError("Cannot upload empty geometry");
		return false;
	}
	glutil::checkGL("Pre geometry upload");
	geometry.renderId = m_models.size();
	m_models.emplace_back(GPUModel());
	GPUModel& model = m_models.back();

	if (!model.vao) glGenVertexArrays(1, &model.vao);
	if (!model.vbo) glGenBuffers(1, &model.vbo);
	if (!model.ebo && !geometry.indices.empty()) glGenBuffers(1, &model.ebo);
	glBindVertexArray(model.vao);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	glBufferData(GL_ARRAY_BUFFER, geometry.vertexData.size(), &geometry.vertexData.front(), GL_STATIC_DRAW);
	// Setup attributes
	for (int i = 0; i < ATTR_MAX; ++i) {
		Geometry::Attribute& attr = geometry.attributes[i];
		if (attr.components) {
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, attr.components, GL_FLOAT, GL_FALSE, geometry.vertexSize, (GLvoid*)(uintptr_t)attr.offset);
		} else {
			glDisableVertexAttribArray(i);
		}
	}
	// Elements
	if (model.ebo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.indices.size() * sizeof(uint), &geometry.indices.front(), GL_STATIC_DRAW);
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

	for (uint i = 0; i < material.map.size(); ++i) {
		if (!material.map[i] || material.tex[i])
			continue;
		Texture tex;
		tex.anisotropy = caps.maxAnisotropy;
		tex.create();
		tex.upload(*material.map[i]);
		material.tex[i] = tex.id;
	}
	return true;
}

void RenderDevice::preRender(const Camera& camera, const std::vector<Light>& lights)
{
	ASSERT(m_env);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	stats = Stats();
	m_program = 0;
	glUseProgram(0);
	m_commonBlock.uniforms.projectionMatrix = camera.projection;
	m_commonBlock.uniforms.viewMatrix = camera.view;
	m_commonBlock.uniforms.cameraPosition = camera.position;
	m_commonBlock.uniforms.globalAmbient = m_env->ambient;
	m_commonBlock.uniforms.exposure = m_env->exposure;

	if (!lights.empty()) {
		uint numLights = std::min((int)lights.size(), MAX_LIGHTS);
		m_commonBlock.uniforms.numLights = numLights;
		for (uint i = 0; i < numLights; i++) {
			const Light& light = lights[i];
			m_lightBlock.uniforms.lights[i].color = light.color;
			m_lightBlock.uniforms.lights[i].position = light.position;
			m_lightBlock.uniforms.lights[i].direction = light.direction;
			m_lightBlock.uniforms.lights[i].params = vec4(light.distance, light.decay, 0.0f, 0.0f);
		}
		m_lightBlock.upload();
	}
	m_commonBlock.upload();
}

void RenderDevice::render(Model& model)
{
	Geometry& geom = *model.geometry;
	Material& mat = *model.material.get();
	if (geom.renderId == -1)
		uploadGeometry(geom);
	GPUModel& gpuData = m_models[geom.renderId];
	if (mat.shaderId < 0)
		uploadMaterial(mat);

	uint programId = m_shaders[mat.shaderId].id;
	if (m_program != programId) {
		m_program = programId;
		glUseProgram(m_program);
		++stats.programs;
	}
	model.updateMatrix();
	m_colorBlock.uniforms.ambient = mat.ambient;
	m_colorBlock.uniforms.diffuse = mat.diffuse;
	m_colorBlock.uniforms.specular = mat.specular;
	m_colorBlock.uniforms.shininess = mat.shininess;
	m_colorBlock.upload();
	m_objectBlock.uniforms.modelMatrix = model.transform;
	mat4 modelView = m_commonBlock.uniforms.viewMatrix * m_objectBlock.uniforms.modelMatrix;
	m_objectBlock.uniforms.modelViewMatrix = modelView;
	m_objectBlock.uniforms.normalMatrix = glm::inverseTranspose(modelView);
	m_objectBlock.upload();
	for (uint i = 0; i < mat.map.size(); ++i) {
		uint tex = mat.tex[i];
		if (!tex) continue;
		glActiveTexture(GL_TEXTURE10 + i);
		glBindTexture(GL_TEXTURE_2D, tex);
		//glUniform1i(i, i);
	}
	glBindVertexArray(gpuData.vao);
	uint mode = mat.tessellate ? GL_PATCHES : GL_TRIANGLES;
	if (gpuData.ebo) {
		glDrawElements(mode, geom.indices.size(), GL_UNSIGNED_INT, 0);
		stats.triangles += geom.indices.size() / 3;
	} else {
		glDrawArrays(mode, 0, geom.numVertices);
		stats.triangles += geom.numVertices / 3;
	}
	++stats.drawCalls;
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

void RenderDevice::postRender()
{
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_shaders[m_shaderNames["postfx"]].id);
	++stats.programs;
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, m_fbo.colorBuffer);
	renderFullscreenQuad();

	glUseProgram(0);
	glutil::checkGL("Post render");
}


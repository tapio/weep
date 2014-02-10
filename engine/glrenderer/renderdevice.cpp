#include "renderdevice.hpp"
#include "glutil.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "light.hpp"

static void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data)
{
	logDebug("OpenGL: %s", msg);
}

RenderDevice::RenderDevice()
{
	logInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	logInfo("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	logInfo("OpenGL Version:  %s", glGetString(GL_VERSION));
	logInfo("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDebugMessageCallback(debugCallback, NULL);

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &caps.maxAnisotropy);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, Engine::width(), Engine::height());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	loadShaders();

	m_commonBlock.create(0);
	m_colorBlock.create(1);
}

void RenderDevice::loadShaders()
{
	m_shaders.clear();
	m_shaderNames.clear();
	std::string err;
	Json jsonShaders = Json::parse(readFile("shaders.json"), err);
	if (!err.empty())
		panic("Failed to read shader config: %s", err.c_str());

	ASSERT(jsonShaders.is_object());
	const Json::object& shaders = jsonShaders.object_items();
	for (auto& it : shaders) {
		const Json& shaderFiles = it.second["shaders"];
		string file;
		m_shaders.emplace_back();
		ShaderProgram& program = m_shaders.back();
		file = shaderFiles["vert"].string_value();
		if (!file.empty()) program.compile(VERTEX_SHADER, readFile(file));
		file = shaderFiles["frag"].string_value();
		if (!file.empty()) program.compile(FRAGMENT_SHADER, readFile(file));
		file = shaderFiles["geom"].string_value();
		if (!file.empty()) program.compile(GEOMETRY_SHADER, readFile(file));
		file = shaderFiles["tesc"].string_value();
		if (!file.empty()) program.compile(TESS_CONTROL_SHADER, readFile(file));
		file = shaderFiles["tese"].string_value();
		if (!file.empty()) program.compile(TESS_EVALUATION_SHADER, readFile(file));

		if (!program.link())
			continue;

		m_shaderNames[it.first] = m_shaders.size() - 1;
		logDebug("Shader \"%s\" initialized (shader mask: %d%d%d%d%d)",
			it.first.c_str(),
			program.has(VERTEX_SHADER),
			program.has(FRAGMENT_SHADER),
			program.has(GEOMETRY_SHADER),
			program.has(TESS_CONTROL_SHADER),
			program.has(TESS_EVALUATION_SHADER)
		);
	}
}

RenderDevice::~RenderDevice()
{
	m_commonBlock.destroy();
	m_colorBlock.destroy();
}

void RenderDevice::destroyModel(Model& model)
{
	if (model.geometry->ebo) {
		glDeleteBuffers(1, &model.geometry->ebo);
		model.geometry->ebo = 0;
	}
	if (model.geometry->vbo) {
		glDeleteBuffers(1, &model.geometry->vbo);
		model.geometry->vbo = 0;
	}
	if (model.geometry->vao) {
		glDeleteVertexArrays(1, &model.geometry->vao);
		model.geometry->vao = 0;
	}
}

bool RenderDevice::uploadGeometry(Geometry& geometry)
{
	if (geometry.vertices.empty()) {
		logError("Cannot upload empty geometry");
		return false;
	}
	glutil::checkGL("Pre geometry upload");

	if (!geometry.vao) glGenVertexArrays(1, &geometry.vao);
	if (!geometry.vbo) glGenBuffers(1, &geometry.vbo);
	if (!geometry.ebo && !geometry.indices.empty()) glGenBuffers(1, &geometry.ebo);
	glBindVertexArray(geometry.vao);
	glBindBuffer(GL_ARRAY_BUFFER, geometry.vbo);
	glBufferData(GL_ARRAY_BUFFER, geometry.vertices.size() * sizeof(Vertex), &geometry.vertices.front(), GL_STATIC_DRAW);
	// Position
	ASSERT(offsetof(Vertex, position) == 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	// TexCoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
	// Normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	// Elements
	if (geometry.ebo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ebo);
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
		return false;
	}
	material.shaderId = it->second;

	if (material.diffuseMap && !material.diffuseTex) {
		Texture tex;
		tex.anisotropy = caps.maxAnisotropy;
		tex.create();
		tex.upload(*material.diffuseMap);
		material.diffuseTex = tex.id;
	}
	return true;
}

void RenderDevice::preRender(const Camera& camera)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	stats = Stats();
	m_program = 0;
	glUseProgram(0);
	m_commonBlock.uniforms.projectionMatrix = camera.projection;
	m_commonBlock.uniforms.viewMatrix = camera.view;
	m_commonBlock.uniforms.cameraPosition = camera.position;
}

void RenderDevice::render(Model& model)
{
	Geometry& geom = *model.geometry;
	Material& mat = *model.material.get();
	if (!geom.vao || !geom.vbo)
		uploadGeometry(geom);
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
	m_colorBlock.upload();
	m_commonBlock.uniforms.modelMatrix = model.transform;
	m_commonBlock.uniforms.modelViewMatrix = m_commonBlock.uniforms.viewMatrix * m_commonBlock.uniforms.modelMatrix;
	m_commonBlock.uniforms.normalMatrix = glm::inverseTranspose(mat3(m_commonBlock.uniforms.modelViewMatrix));
	m_commonBlock.upload();
	if (mat.diffuseTex) {
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, mat.diffuseTex);
		//glUniform1i(0, 0);
	}
	glBindVertexArray(geom.vao);
	uint mode = mat.tessellate ? GL_PATCHES : GL_TRIANGLES;
	if (geom.ebo) {
		glDrawElements(mode, geom.indices.size(), GL_UNSIGNED_INT, 0);
		stats.triangles += geom.indices.size() / 3;
	} else {
		glDrawArrays(mode, 0, geom.vertices.size());
		stats.triangles += geom.vertices.size() / 3;
	}
	++stats.drawCalls;
	glBindVertexArray(0);
	glutil::checkGL("Post draw");
}

void RenderDevice::postRender()
{
	glBindVertexArray(0);
	glUseProgram(0);
	glutil::checkGL("Post render");
}


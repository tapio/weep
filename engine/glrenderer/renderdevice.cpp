#include "renderdevice.hpp"
#include "glutil.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "camera.hpp"

RenderDevice::RenderDevice()
{
	logInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	logInfo("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	logInfo("OpenGL Version:  %s", glGetString(GL_VERSION));
	logInfo("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ShaderProgram shader;
	shader.compile(VERTEX_SHADER, readFile("shaders/core.vert"));
	shader.compile(FRAGMENT_SHADER, readFile("shaders/core.frag"));
	shader.link();
	shaders.push_back(shader.id);

	commonBlock.create(0);
	colorBlock.create(1);
}


RenderDevice::~RenderDevice()
{
	commonBlock.destroy();
	colorBlock.destroy();
}

bool RenderDevice::uploadModel(Model& model)
{
	bool success = uploadMaterial(*model.material);
	if (!success) return false;
	return uploadGeometry(*model.geometry);
}

void RenderDevice::destroyModel(Model& model)
{
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

	glutil::checkGL("Post geometry upload");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;
}

bool RenderDevice::uploadMaterial(Material& material)
{
	material.shaderId = shaders.front();
	if (material.diffuseMap && !material.diffuseTex) {
		Texture tex;
		tex.create();
		tex.upload(*material.diffuseMap);
		material.diffuseTex = tex.id;
	}
	return true;
}

void RenderDevice::preRender(Camera& camera)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	program = 0;
	glUseProgram(0);
	commonBlock.uniforms.projectionMatrix = camera.projection;
	commonBlock.uniforms.viewMatrix = camera.view;

}

void RenderDevice::render(Model& model)
{
	Geometry& geom = *model.geometry.get();
	Material& mat = *model.material.get();
	ASSERT(geom.vao && geom.vbo);
	if (program != mat.shaderId) {
		program = mat.shaderId;
		glUseProgram(program);
	}
	colorBlock.uniforms.ambient = mat.ambient;
	colorBlock.uniforms.diffuse = mat.diffuse;
	colorBlock.uniforms.specular = mat.specular;
	colorBlock.upload();
	commonBlock.uniforms.modelMatrix = model.transform;
	commonBlock.uniforms.modelViewMatrix = commonBlock.uniforms.modelMatrix * commonBlock.uniforms.viewMatrix;
	commonBlock.upload();
	if (mat.diffuseTex) {
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, mat.diffuseTex);
		//glUniform1i(0, 0);
	}
	glBindVertexArray(geom.vao);
	glDrawArrays(GL_TRIANGLES, 0, geom.vertices.size());
	glutil::checkGL("Post draw");
}

void RenderDevice::postRender()
{
	glBindVertexArray(0);
	glUseProgram(0);
	glutil::checkGL("Post render");
}


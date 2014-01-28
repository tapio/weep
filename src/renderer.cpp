#include "renderer.hpp"
#include "shader.hpp"
#include "geometry.hpp"
#include "platform.hpp"

#include <GL/glcorearb.h>

namespace
{
	ShaderProgram shader;
}

Renderer::Renderer()
{
	logInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	logInfo("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	logInfo("OpenGL Version:  %s", glGetString(GL_VERSION));
	logInfo("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	shader.compile(VERTEX_SHADER, readFile("shaders/core.vert"));
	shader.compile(VERTEX_SHADER, readFile("shaders/core.frag"));
	shader.link();
	shader.use();
}

Renderer::~Renderer()
{
}

void Renderer::addGeometry(Geometry* geometry)
{
	geometry->upload();
	geometries.push_back(geometry);
}

void Renderer::render()
{
	for (auto geom : geometries) {
		glBindVertexArray(geom->vao);
		glDrawArrays(GL_TRIANGLES, 0, geom->vertices.size());
	}
	glBindVertexArray(0);
}



#include "renderer.hpp"
#include "shader.hpp"
#include "geometry.hpp"
#include "platform.hpp"
#include "glutil.hpp"

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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	shader.compile(VERTEX_SHADER, readFile("shaders/core.vert"));
	shader.compile(FRAGMENT_SHADER, readFile("shaders/core.frag"));
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto geom : geometries) {
		ASSERT(geom->vao && geom->vbo);
		glBindVertexArray(geom->vao);
		glDrawArrays(GL_TRIANGLES, 0, geom->vertices.size());
		glutil::checkGL("Post draw");
	}
	glBindVertexArray(0);
	glutil::checkGL("Post render");
}



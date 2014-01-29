#include "renderer.hpp"
#include "shader.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "platform.hpp"
#include "glutil.hpp"

Renderer* Renderer::instance = nullptr;
void Renderer::create() { instance = new Renderer(); }
void Renderer::destroy() { if (instance) delete instance; }


Renderer::Renderer()
{
	logInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	logInfo("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	logInfo("OpenGL Version:  %s", glGetString(GL_VERSION));
	logInfo("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

Renderer::~Renderer()
{
}

void Renderer::addModel(std::shared_ptr<Model> model)
{
	model->geometry->upload();
	models.push_back(model);
}

void Renderer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	uint program = 0;
	for (auto model : models) {
		Geometry& geom = *model->geometry.get();
		Material& mat = *model->material.get();
		ASSERT(geom.vao && geom.vbo);
		if (program != mat.shaderId) {
			program = mat.shaderId;
			glUseProgram(program);
		}
		glBindVertexArray(geom.vao);
		glDrawArrays(GL_TRIANGLES, 0, geom.vertices.size());
		glutil::checkGL("Post draw");
	}
	glBindVertexArray(0);
	glUseProgram(0);
	glutil::checkGL("Post render");
}



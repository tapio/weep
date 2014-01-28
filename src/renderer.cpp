#include "renderer.hpp"
#include "shader.hpp"
#include "platform.hpp"

namespace
{
	ShaderProgram shader;
}

Renderer::Renderer()
{
	shader.compile(VERTEX_SHADER, Platform::readFile("shaders/core.vert"));
	shader.compile(VERTEX_SHADER, Platform::readFile("shaders/core.frag"));
	shader.link();
}

Renderer::~Renderer()
{
}

void Renderer::render()
{

}



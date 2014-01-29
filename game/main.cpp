#include "platform.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "model.hpp"
#include "renderer.hpp"

int main(int argc, char* argv[])
{
	Platform::init();

	ShaderProgram shader;
	shader.compile(VERTEX_SHADER, readFile("shaders/core.vert"));
	shader.compile(FRAGMENT_SHADER, readFile("shaders/core.frag"));
	shader.link();

	Model model;
	model.geometry = std::shared_ptr<Geometry>(new Geometry());
	*model.geometry = Geometry::createPlane(100, 100);
	model.material = std::shared_ptr<Material>(new Material());
	model.material->shaderId = shader.id;

	GetRenderer().addModel(&model);

	Platform::run();
	Platform::deinit();

	return 0;
}

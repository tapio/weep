#include "platform.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "model.hpp"
#include "renderer.hpp"

int main(int argc, char* argv[])
{
	Platform::init();

	Geometry geom = Geometry::createPlane(100, 100);

	ShaderProgram shader;
	shader.compile(VERTEX_SHADER, readFile("shaders/core.vert"));
	shader.compile(FRAGMENT_SHADER, readFile("shaders/core.frag"));
	shader.link();

	Material mat;
	mat.shaderId = shader.id;

	Model model;
	model.geometry = std::shared_ptr<Geometry>(&geom);
	model.material = std::shared_ptr<Material>(&mat);
	GetRenderer().addModel(&model);

	Platform::run();
	Platform::deinit();

	return 0;
}

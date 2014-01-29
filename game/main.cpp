#include "engine.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "model.hpp"
#include "image.hpp"
#include "renderer.hpp"
#include "glrenderer/shader.hpp"

int main(int argc, char* argv[])
{
	Engine::init();

	Image image;
	image.load("../data/debug/uvtestgrid.png", 4);

	Model model;
	model.geometry = std::shared_ptr<Geometry>(new Geometry());
	*model.geometry = Geometry::createPlane(1, 1);
	model.material = std::shared_ptr<Material>(new Material());
	model.material->ambient = vec3(0.2f, 0.2f, 0.2f);
	model.material->diffuse = vec3(0.0f, 0.0f, 0.3f);

	GetRenderer().addModel(&model);

	Engine::run();
	Engine::deinit();

	return 0;
}

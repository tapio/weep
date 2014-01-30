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

	Model model;
	model.geometry.reset(new Geometry());
	*model.geometry = Geometry::createPlane(1, 1);
	model.material.reset(new Material());
	model.material->ambient = vec3(0.2f, 0.2f, 0.2f);
	model.material->diffuse = vec3(0.0f, 0.0f, 0.3f);
	model.material->diffuseMap.reset(new Image("../data/debug/uvtestgrid.png", 4));
	model.transform = translate(model.transform, vec3(0.1f, 0.1f, -10.0f));

	GetRenderer().addModel(&model);

	Engine::run();
	Engine::deinit();

	return 0;
}

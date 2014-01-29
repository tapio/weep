#include "platform.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "glrenderer/shader.hpp"

int main(int argc, char* argv[])
{
	Platform::init();

	Model model;
	model.geometry = std::shared_ptr<Geometry>(new Geometry());
	*model.geometry = Geometry::createPlane(1, 1);
	model.material = std::shared_ptr<Material>(new Material());

	GetRenderer().addModel(&model);

	Platform::run();
	Platform::deinit();

	return 0;
}

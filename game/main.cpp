#include "engine.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "model.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "glrenderer/shader.hpp"

int main(int, char*[])
{
	Engine::init();

	Renderer renderer;

	Camera camera;
	camera.makePerspective(45, 1280.0f / 720.0f, 0.1, 1000);
	//camera.view = lookAt(vec3(0, 1, 0), vec3(0, 0, 0), vec3(0, 1, 0));

	Model model;
	model.geometry.reset(new Geometry());
	*model.geometry = Geometry::createPlane(1, 1);
	model.material.reset(new Material());
	model.material->ambient = vec3(0.2f, 0.2f, 0.2f);
	model.material->diffuse = vec3(0.0f, 0.0f, 0.3f);
	model.material->diffuseMap.reset(new Image("../data/debug/uvtestgrid.png", 4));
	model.transform = translate(model.transform, vec3(0.1f, 0.1f, -10.0f));

	renderer.addModel(&model);

	Engine::run([&renderer, &camera]() {
		renderer.render(camera);
	});

	Engine::deinit();

	return 0;
}

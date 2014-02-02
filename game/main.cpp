#include "engine.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"

#include <SDL2/SDL.h>


int main(int, char*[])
{
	Engine::init();
	Resources resources;

	Camera camera;
	camera.makePerspective(45, 1280.0f / 720.0f, 0.1, 1000);
	//camera.view = lookAt(vec3(0, 1, 0), vec3(0, 0, 0), vec3(0, 1, 0));

	Model model;
	model.geometry.reset(new Geometry());
	*model.geometry = Geometry::createPlane(1, 1);
	model.material.reset(new Material());
	model.material->ambient = vec3(0.2f, 0.2f, 0.2f);
	model.material->diffuse = vec3(0.0f, 0.0f, 0.3f);
	model.material->diffuseMap = resources.getImage("data/debug/uvtestgrid.png");
	model.transform = translate(model.transform, vec3(0.1f, 0.1f, -10.0f));

	Renderer renderer;
	renderer.addModel(&model);

	bool running = true;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
			{
				running = false;
				break;
			}
		}

		renderer.render(camera);
		Engine::swap();
	}

	Engine::deinit();

	return EXIT_SUCCESS;
}

#include "engine.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"

#include <SDL2/SDL.h>


int main(int, char*[])
{
	Engine::init();
	Resources resources;
	Renderer renderer;

	float ar = Engine::width() / (float)Engine::height();
	Camera camera;
	camera.makePerspective(45, ar, 0.1, 1000);
	camera.view = glm::lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0));

	Controller controller(camera.position, camera.rotation);

	const string scenePath = "testscene.json";
	Scene scene;
	scene.load(scenePath, resources);

	bool running = true;
	bool active = false;
	SDL_Event e;
	while (running) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
				break;
			}

			if (e.type == SDL_KEYUP) {
				SDL_Keysym keysym = e.key.keysym;

				if (keysym.sym == SDLK_ESCAPE) {
					if (active) {
						active = false;
						Engine::grabMouse(false);
					} else running = false;
					break;
				}

				if (keysym.mod == KMOD_LCTRL && keysym.sym == SDLK_r) {
					renderer.reset(scene);
					scene.reset();
					resources.reset();
					scene.load(scenePath, resources);
					continue;
				}
				if (keysym.mod == KMOD_LCTRL && keysym.sym == SDLK_d) {
					renderer.dumpStats();
					continue;
				}
			}

			if (e.type == SDL_MOUSEBUTTONDOWN && !active) {
				active = true;
				Engine::grabMouse(true);
			}
			else if (e.type == SDL_MOUSEMOTION && active) {
				const vec3 yaxis(0, 1, 0);
				controller.rotation = glm::rotate(controller.rotation, -0.005f * e.motion.xrel, yaxis);
			}
		}

		controller.update(0.01667f); // TODO: Proper frame time

		renderer.render(scene, camera);

		Engine::swap();
	}

	renderer.reset(scene);
	Engine::deinit();

	return EXIT_SUCCESS;
}

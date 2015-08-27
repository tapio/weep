#include "engine.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"

#include <SDL2/SDL.h>


int main(int, char*[])
{
	Resources resources;
	resources.addPath("../data/");
	Engine::init(resources.findPath("settings.json"));
	Renderer renderer(resources);

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
				else if (keysym.sym == SDLK_F1) {
					renderer.dumpStats();
					logDebug("Last frame took %fms", Engine::dt * 1000);
					continue;
				}
				else if (keysym.sym == SDLK_F2) {
					renderer.toggleWireframe();
					continue;
				}
			}

			if (e.type == SDL_MOUSEBUTTONDOWN && !active) {
				active = true;
				Engine::grabMouse(true);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && active) {
				active = false;
				Engine::grabMouse(false);
			}
			else if (e.type == SDL_MOUSEMOTION && active) {
				controller.rotation = glm::rotate(controller.rotation, -0.005f * e.motion.xrel, yaxis);
				controller.rotation = glm::rotate(controller.rotation, -0.005f * e.motion.yrel, xaxis);
			}
		}

		controller.update(Engine::dt);

		scene.getLights()[0].position.x = 5.f * glm::sin(SDL_GetTicks() / 800.f);
		scene.getLights()[1].position.x = 4.f * glm::sin(SDL_GetTicks() / 500.f);
		scene.getLights()[2].position.y = 2.f + 1.5f * glm::sin(SDL_GetTicks() / 500.f);

		renderer.render(scene, camera);

		Engine::swap();
	}

	renderer.reset(scene);
	Engine::deinit();

	return EXIT_SUCCESS;
}

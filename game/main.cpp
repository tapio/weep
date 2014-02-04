#include "engine.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"

#include <SDL2/SDL.h>


int main(int, char*[])
{
	Engine::init();
	Resources resources;
	Renderer renderer;

	float ar = Engine::width() / (float)Engine::height();
	Camera camera;
	camera.makePerspective(45, ar, 0.1, 1000);
	camera.view = lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0));

	const string scenePath = "testscene.json";
	Scene scene;
	scene.load(scenePath, resources);

	bool running = true;
	bool active = false;
	SDL_Event e;
	while (running) {
		renderer.render(scene, camera);
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
			}

			if (e.type == SDL_KEYDOWN) {
				SDL_Keysym keysym = e.key.keysym;

				vec3 input;
				switch (keysym.scancode) {
					case SDL_SCANCODE_UP:
					case SDL_SCANCODE_W:
						input.z = -1;
						break;
					case SDL_SCANCODE_DOWN:
					case SDL_SCANCODE_S:
						input.z = 1;
						break;
					case SDL_SCANCODE_LEFT:
					case SDL_SCANCODE_A:
						input.x = -1;
						break;
					case SDL_SCANCODE_RIGHT:
					case SDL_SCANCODE_D:
						input.x = 1;
						break;
					case SDL_SCANCODE_PAGEUP:
					case SDL_SCANCODE_Q:
						input.y = 1;
						break;
					case SDL_SCANCODE_PAGEDOWN:
					case SDL_SCANCODE_Z:
						input.y = -1;
						break;
					default:
						break;
				}
				vec3 dir = camera.rotation * input;
				camera.position += dir * 0.1f;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && !active) {
				active = true;
				Engine::grabMouse(true);
			}
			else if (e.type == SDL_MOUSEMOTION && active) {
				const vec3 xaxis(1, 0, 0), yaxis(0, 1, 0);
				camera.rotation = rotate(camera.rotation, -0.005f * e.motion.xrel, yaxis);
			}

		}
		Engine::swap();
	}

	renderer.reset(scene);
	Engine::deinit();

	return EXIT_SUCCESS;
}

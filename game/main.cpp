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

	Scene scene;
	scene.load("testscene.json", resources);

	bool running = true;
	SDL_Event e;
	while (running) {
		renderer.render(scene, camera);
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
			{
				running = false;
				break;
			}

			if (e.type == SDL_KEYDOWN) {
				vec3 input;
				switch (e.key.keysym.scancode) {
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
			else if (e.type == SDL_MOUSEMOTION) {
				const vec3 axis(0, 1, 0);
				camera.rotation = rotate(camera.rotation, -0.01f * e.motion.xrel, axis);
			}

		}
		Engine::swap();
	}

	renderer.reset();
	Engine::deinit();

	return EXIT_SUCCESS;
}

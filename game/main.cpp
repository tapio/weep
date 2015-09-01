#include "engine.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"

#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"


int main(int, char*[])
{
	Resources resources;
	resources.addPath("../data/");
	Engine::init(resources.findPath("settings.json"));
	if (Engine::settings["moddir"].is_string())
		resources.addPath(Engine::settings["moddir"].string_value());
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
		ImGui_ImplSDLGL3_NewFrame();

		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDLGL3_ProcessEvent(&e);
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
				else if (keysym.sym == SDLK_F3) {
					Engine::vsync(!Engine::vsync());
					continue;
				}
				else if (keysym.sym == SDLK_F4) {
					static bool removeModDir = true;
					if (Engine::settings["moddir"].is_string()) {
						if (removeModDir) resources.removePath(Engine::settings["moddir"].string_value());
						else resources.addPath(Engine::settings["moddir"].string_value());
						removeModDir = !removeModDir;
					}
					continue;
				}
			}

			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
				active = !active;
				Engine::grabMouse(active);
			}
			else if (e.type == SDL_MOUSEMOTION && active) {
				controller.angles.x += -0.05f * e.motion.yrel;
				controller.angles.y += -0.05f * e.motion.xrel;
			}
		}

		controller.update(Engine::dt);

		auto& lights = scene.getLights();
		lights[0].position.x = 5.f * glm::sin(Engine::timems() / 800.f);
		lights[1].position.x = 4.f * glm::sin(Engine::timems() / 500.f);
		lights[2].position.y = 1.f + 1.5f * glm::sin(Engine::timems() / 1000.f);

		for (uint i = 0; i < lights.size(); ++i) {
			Model* model = scene.find("light_0" + std::to_string(i+1));
			if (model) {
				Light& light = lights[i];
				model->position = light.position;
				//model->material->ambient = light.color;
			}
		}

		renderer.render(scene, camera);

		ImGui::Text("Right mouse button to toggle mouse grab.");
		ImGui::Text("FPS: %d (%fms)", int(1.0 / Engine::dt), Engine::dt);

		Environment& env = renderer.env();
		ImGui::ColorEdit3("Ambient", (float*)&env.ambient);
		ImGui::SliderFloat("Exposure", &env.exposure, 0.0f, 10.0f);

		//ImGui::ShowTestWindow();
		//ImGui::ShowStyleEditor();

		ImGui::Render();

		Engine::swap();
	}

	renderer.reset(scene);
	Engine::deinit();

	return EXIT_SUCCESS;
}

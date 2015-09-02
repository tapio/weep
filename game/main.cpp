#include "engine.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"
#include "physics.hpp"
#include "glrenderer/renderdevice.hpp"

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

	char scenePath[128] = "testscene.json";
	Scene scene;
	scene.load(scenePath, resources);

	PhysicsSystem physics;
	physics.addScene(scene);

	ImGui_ImplSDLGL3_Init(Engine::window);

	bool running = true;
	bool active = false;
	bool reload = false;
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
					reload = true;
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

		// Calculate and send current velocity from controller to Bullet
		vec3 p0 = camera.position;
		controller.update(Engine::dt);
		vec3 p1 = camera.position;
		vec3 vel = float(1.0f / Engine::dt) * (p1 - p0);

		Model* cameraObj = scene.find("camera");
		if (cameraObj) {
			cameraObj->body->setSleepingThresholds(0.f, 0.f); // TODO: Not every frame
			cameraObj->body->setGravity(btVector3(0, 0, 0)); // TODO: Not every frame
			btTransform trans(convert(camera.rotation), convert(camera.position));
			cameraObj->body->setWorldTransform(trans);
			cameraObj->body->setLinearVelocity(convert(vel));
		}

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

		// Physics
		float physTimeMs = 0.f;
		{
			uint64 t0 = SDL_GetPerformanceCounter();
			physics.step(Engine::dt);
			physics.syncTransforms(scene);
			uint64 t1 = SDL_GetPerformanceCounter();
			physTimeMs = (t1 - t0) / (double)SDL_GetPerformanceFrequency() * 1000.0;
		}

		if (cameraObj) {
			const btTransform& trans = cameraObj->body->getCenterOfMassTransform();
			camera.position = convert(trans.getOrigin());
			camera.rotation = convert(trans.getRotation());
		}

		renderer.render(scene, camera);

		ImGui::Text("Right mouse button to toggle mouse grab.");
		ImGui::Text("FPS: %d (%.3fms)", int(1.0 / Engine::dt), Engine::dt * 1000.f);
		ImGui::Text("Cam: %.1f %.1f %.1f", camera.position.x, camera.position.y, camera.position.z);
		if (ImGui::CollapsingHeader("Stats")) {
			ImGui::Text("Physics:    %.3fms", physTimeMs);
			const RenderDevice::Stats& stats = renderer.device().stats;
			ImGui::Text("Triangles:  %d", stats.triangles);
			ImGui::Text("Programs:   %d", stats.programs);
			ImGui::Text("Draw calls: %d", stats.drawCalls);
		}
		if (ImGui::CollapsingHeader("Settings")) {
			bool vsync = Engine::vsync();
			bool oldVsync = vsync;
			ImGui::Checkbox("V-sync", &vsync);
			if (vsync != oldVsync)
				Engine::vsync(vsync);
		}
		if (ImGui::CollapsingHeader("Environment")) {
			Environment& env = renderer.env();
			ImGui::SliderFloat("Exposure", &env.exposure, 0.0f, 10.0f);
			ImGui::ColorEdit3("Ambient", (float*)&env.ambient);
			ImGui::ColorEdit3("Sun Color", (float*)&env.sunColor);
			ImGui::SliderFloat3("Sun Dir", (float*)&env.sunDirection, -3.f, 3.f);
		}
		if (ImGui::CollapsingHeader("Scene")) {
			ImGui::InputText("", scenePath, sizeof(scenePath));
			ImGui::SameLine();
			if (ImGui::Button("Load")) {
				reload = true;
			}
		}

		//ImGui::ShowTestWindow();
		//ImGui::ShowStyleEditor();

		ImGui::Render();

		Engine::swap();

		if (reload) {
			renderer.reset(scene);
			physics.reset();
			scene.reset();
			resources.reset();
			scene.load(scenePath, resources);
			physics.addScene(scene);
			reload = false;
		}
	}

	ImGui_ImplSDLGL3_Shutdown();
	renderer.reset(scene);
	Engine::deinit();

	return EXIT_SUCCESS;
}

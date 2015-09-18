#include "engine.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "glrenderer/renderdevice.hpp"

#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"

void SetupImGuiStyle();

int main(int, char*[])
{
	Resources resources;
	resources.addPath("../data/");
	Engine::init(resources.findPath("settings.json"));
	if (Engine::settings["moddir"].is_string())
		resources.addPath(Engine::settings["moddir"].string_value());
	Renderer renderer(resources);
	AudioSystem audio;

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
	SetupImGuiStyle();

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

		Entity cameraEnt = scene.world.get_entity_by_tag("camera");
		if (cameraEnt.is_alive()) {
			btRigidBody& cameraBody = cameraEnt.get<btRigidBody>();
			btTransform trans(convert(camera.rotation), convert(camera.position));
			cameraBody.setWorldTransform(trans);
			cameraBody.setLinearVelocity(convert(vel));
		}

		for (uint i = 0; i < 3; ++i) {
			Entity e = scene.world.get_entity_by_tag("light_0" + std::to_string(i+1));
			if (!e.is_alive())
				continue;
			Light& light = e.get<Light>();
			/**/ if (i == 0) light.position.x = 5.f * glm::sin(Engine::timems() / 800.f);
			else if (i == 1) light.position.x = 4.f * glm::sin(Engine::timems() / 500.f);
			else if (i == 2) light.position.y = 1.f + 1.5f * glm::sin(Engine::timems() / 1000.f);
			if (e.has<Model>()) {
				e.get<Model>().position = light.position;
				//e.get<Model>().material->ambient = light.color;
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

		if (cameraEnt.is_alive()) {
			const btTransform& trans = cameraEnt.get<btRigidBody>().getCenterOfMassTransform();
			camera.position = convert(trans.getOrigin());
			camera.rotation = convert(trans.getRotation());
		}

		// Audio
		float audioTimeMs = 0.f;
		{
			uint64 t0 = SDL_GetPerformanceCounter();
			audio.update(camera);
			uint64 t1 = SDL_GetPerformanceCounter();
			audioTimeMs = (t1 - t0) / (double)SDL_GetPerformanceFrequency() * 1000.0;
		}

		// Graphics
		float renderTimeMs = 0.f;
		{
			uint64 t0 = SDL_GetPerformanceCounter();
			renderer.render(scene, camera);
			uint64 t1 = SDL_GetPerformanceCounter();
			renderTimeMs = (t1 - t0) / (double)SDL_GetPerformanceFrequency() * 1000.0;
		}

		ImGui::Text("Right mouse button to toggle mouse grab.");
		ImGui::Text("FPS: %d (%.3fms)", int(1.0 / Engine::dt), Engine::dt * 1000.f);
		ImGui::Text("Cam: %.1f %.1f %.1f", camera.position.x, camera.position.y, camera.position.z);
		if (ImGui::CollapsingHeader("Stats")) {
			ImGui::Text("Physics:      %.3fms", physTimeMs);
			ImGui::Text("Audio:        %.3fms", audioTimeMs);
			ImGui::Text("CPU Render:   %.3fms", renderTimeMs);
			const RenderDevice::Stats& stats = renderer.device().stats;
			ImGui::Text("Triangles:    %d", stats.triangles);
			ImGui::Text("Programs:     %d", stats.programs);
			ImGui::Text("Draw calls:   %d", stats.drawCalls);
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
			ImGui::SliderInt("Tonemap", (int*)&env.tonemap, 0, Environment::TONEMAP_COUNT-1);
			ImGui::SliderFloat("Exposure", &env.exposure, 0.0f, 10.0f);
			ImGui::SliderFloat("Bloom Threshold", &env.bloomThreshold, 0.0f, 2.0f);
			ImGui::SliderFloat("Bloom Intensity", &env.bloomIntensity, 1.0f, 10.0f);
			ImGui::ColorEdit3("Ambient", (float*)&env.ambient);
			ImGui::ColorEdit3("Sun Color", (float*)&env.sunColor);
			ImGui::SliderFloat3("Sun Dir", (float*)&env.sunDirection, -3.f, 3.f);
			ImGui::ColorEdit3("Fog Color", (float*)&env.fogColor);
			ImGui::SliderFloat("Fog Density", &env.fogDensity, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Scene")) {
			ImGui::InputText("", scenePath, sizeof(scenePath));
			ImGui::SameLine();
			if (ImGui::Button("Load")) {
				reload = true;
			}
			const char* prefabs[scene.prefabs.size()];
			int temp = 0;
			for (auto it : scene.prefabs)
				prefabs[temp++] = it.first.c_str();
			static int selectedPrefab = 0;
			ImGui::Combo("", &selectedPrefab, prefabs, scene.prefabs.size());
			bool create = false;
			vec3 vel(0, 0, 0);
			ImGui::SameLine();
			if (ImGui::Button("Drop")) {
				create = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Shoot")) {
				create = true;
				vel = glm::rotate(camera.rotation, vec3(0, 0, -20));
			}
			if (create) {
				Entity e = scene.instantiate(scene.prefabs[prefabs[selectedPrefab]], resources);
				vec3 pos = camera.position + glm::rotate(camera.rotation, vec3(0, 0, -2));
				if (e.has<Model>()) {
					Model& model = e.get<Model>();
					model.position = pos;
					model.rotation = camera.rotation;
				}
				if (e.has<btRigidBody>()) {
					physics.add(e);
					btRigidBody& body = e.get<btRigidBody>();
					btTransform trans(convert(camera.rotation), convert(pos));
					body.setWorldTransform(trans);
					body.setLinearVelocity(convert(vel));
				}
			}
		}

		//ImGui::ShowTestWindow();
		//ImGui::ShowStyleEditor();

		ImGui::Render();

		Engine::swap();

		scene.world.update();

		if (reload) {
			renderer.reset(scene);
			physics.reset();
			scene.reset();
			resources.reset();
			renderer.device().loadShaders();
			renderer.env().reset();
			renderer.env().load("environment.json", resources);
			renderer.device().setEnvironment(&renderer.env());
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

void SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 6.f;
	style.FrameRounding = 4.f;
	style.ScrollbarRounding = 5.f;
	style.GrabRounding = 3.f;
	style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.50f, 0.86f, 1.00f, 0.45f);
	style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.78f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.50f, 0.75f, 1.00f, 0.55f);
	style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.40f, 0.55f, 0.55f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.13f, 0.13f, 0.13f, 0.67f);
	style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.27f, 0.27f, 0.27f, 0.67f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.40f, 0.40f, 0.40f, 0.67f);
	style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.53f, 0.53f, 0.53f, 0.67f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.53f, 0.53f, 0.53f, 0.67f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.38f, 0.00f, 0.67f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.44f, 0.59f, 0.00f, 0.67f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.09f, 0.19f, 0.00f, 0.67f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.40f, 0.78f, 0.90f, 0.45f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.45f, 0.78f, 0.90f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.53f, 0.71f, 0.78f, 0.80f);
	style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.50f, 0.78f, 0.90f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.70f, 0.78f, 0.90f, 0.60f);
}

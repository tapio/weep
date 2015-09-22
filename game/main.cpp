#include "engine.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "module.hpp"
#include "glrenderer/renderdevice.hpp"
#include "game.hpp"

#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"

void SetupImGuiStyle();

void init(Game& game, Scene& scene, const string& scenePath)
{
	game.entities = Entities();
	game.entities.add_system<RenderSystem>(game.resources);
	game.entities.add_system<PhysicsSystem>();
	game.entities.add_system<AudioSystem>();
	scene = Scene(game.entities);
	scene.load(scenePath, game.resources);
	Entity cameraEnt = scene.world->get_entity_by_tag("camera");
	Camera& camera = cameraEnt.get<Camera>();
	cameraEnt.add<Controller>(camera.position, camera.rotation);
	if (cameraEnt.has<btRigidBody>())
		cameraEnt.get<Controller>().body = &cameraEnt.get<btRigidBody>();
}

int main(int, char*[])
{
	Game game;
	Resources& resources = game.resources;
	resources.addPath("../data/");
	Engine::init(resources.findPath("settings.json"));
	if (Engine::settings["moddir"].is_string())
		resources.addPath(Engine::settings["moddir"].string_value());

	Scene scene(game.entities);

	char scenePath[128] = "testscene.json";
	init(game, scene, scenePath);

	ImGui_ImplSDLGL3_Init(Engine::window);
	SetupImGuiStyle();

	Modules modules;
	modules.load(Engine::settings["modules"]);
	modules.call($id(INIT), &game);

	bool running = true;
	bool active = false;
	bool reload = false;
	SDL_Event e;
	while (running) {
		ImGui_ImplSDLGL3_NewFrame();

		RenderSystem& renderer = game.entities.get_system<RenderSystem>();
		PhysicsSystem& physics = game.entities.get_system<PhysicsSystem>();
		AudioSystem& audio = game.entities.get_system<AudioSystem>();
		Entity cameraEnt = game.entities.get_entity_by_tag("camera");
		Controller& controller = cameraEnt.get<Controller>();
		Camera& camera = cameraEnt.get<Camera>();

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

		// Modules
		float moduleTimeMs = 0.f;
		{
			uint64 t0 = SDL_GetPerformanceCounter();
			modules.call($id(UPDATE), &game);
			uint64 t1 = SDL_GetPerformanceCounter();
			moduleTimeMs = (t1 - t0) / (double)SDL_GetPerformanceFrequency() * 1000.0;
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

		if (cameraEnt.has<btRigidBody>()) {
			btRigidBody& body = cameraEnt.get<btRigidBody>();
			camera.position = convert(body.getCenterOfMassPosition());
			controller.onGround = physics.testGroundHit(body);
		} else {
			camera.position = controller.position;
		}
		camera.rotation = controller.rotation;

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
		if (ImGui::CollapsingHeader("Stats")) {
			ImGui::Text("Physics:      %.3fms", physTimeMs);
			ImGui::Text("Audio:        %.3fms", audioTimeMs);
			ImGui::Text("CPU Render:   %.3fms", renderTimeMs);
			ImGui::Text("Module upd:   %.3fms", moduleTimeMs);
			const RenderDevice::Stats& stats = renderer.device().stats;
			ImGui::Text("Lights:       %d", stats.lights);
			ImGui::Text("Triangles:    %d", stats.triangles);
			ImGui::Text("Programs:     %d", stats.programs);
			ImGui::Text("Draw calls:   %d", stats.drawCalls);
		}
		if (ImGui::CollapsingHeader("Camera")) {
			ImGui::Text("Position: %.1f %.1f %.1f", camera.position.x, camera.position.y, camera.position.z);
			if (ImGui::Checkbox("Fly", &controller.fly)) {
				Entity cameraEnt = game.entities.get_entity_by_tag("camera");
				if (cameraEnt.is_alive() && cameraEnt.has<btRigidBody>()) {
					btRigidBody& body = cameraEnt.get<btRigidBody>();
					body.setGravity(controller.fly ? btVector3(0, 0, 0) : physics.dynamicsWorld->getGravity());
				}
			}
			ImGui::SliderFloat("Move force", &controller.moveForce, 0.0f, 10000.0f);
			ImGui::SliderFloat("Brake force", &controller.brakeForce, 0.0f, 10000.0f);
			ImGui::SliderFloat("Jump force", &controller.jumpForce, 0.0f, 10000.0f);
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
		if (ImGui::CollapsingHeader("Modules")) {
			if (ImGui::Button("Reload all##Modules")) {
				modules.load(Engine::settings["modules"]);
				modules.call($id(INIT), &game);
			}
			ImGui::Text("Active modules:");
			for (auto& it : modules) {
				ImGui::Checkbox(it.first.c_str(), &it.second.enabled);
				ImGui::SameLine();
				if (ImGui::Button(("Reload##" + it.first).c_str()))
					modules.reload(it.first);
			}
		}
		if (ImGui::CollapsingHeader("Scene")) {
			ImGui::InputText("", scenePath, sizeof(scenePath));
			ImGui::SameLine();
			if (ImGui::Button("Load##Scene"))
				reload = true;
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
			if (ImGui::Button("Generate skyrunner level")) {
				modules.call("skyrunner_levelgen", $id(GENERATE_LEVEL), &game);
			}
		}

		//ImGui::ShowTestWindow();
		//ImGui::ShowStyleEditor();

		ImGui::Render();

		Engine::swap();

		game.entities.update();

		if (reload) {
			modules.call($id(DEINIT), &game);
			renderer.reset(scene);
			scene.reset();
			resources.reset();
			init(game, scene, scenePath);
			modules.call($id(INIT), &game);
			reload = false;
		}
	}

	ImGui_ImplSDLGL3_Shutdown();

	game.entities.remove_system<AudioSystem>();
	game.entities.remove_system<PhysicsSystem>();
	game.entities.remove_system<RenderSystem>();

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

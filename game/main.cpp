#include "engine.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "components.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "controller.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "module.hpp"
#include "gui.hpp"
#include "glrenderer/renderdevice.hpp"
#include "game.hpp"
#include <SDL2/SDL.h>

void init(Game& game, SceneLoader& scene, const string& scenePath)
{
	game.entities = Entities();
	game.entities.add_system<RenderSystem>(game.resources);
	game.entities.add_system<PhysicsSystem>();
	game.entities.add_system<AudioSystem>();
	game.entities.add_system<ModuleSystem>();
	game.entities.get_system<ModuleSystem>().load(Engine::settings["modules"], false);
	game.entities.add_system<ImGuiSystem>(game.engine.window);
	game.entities.get_system<ImGuiSystem>().applyDefaultStyle();
	scene = SceneLoader(game.entities);
	scene.load(scenePath, game.resources);
	Entity cameraEnt = game.entities.get_entity_by_tag("camera");
	ASSERT(cameraEnt.is_alive());
	Camera& camera = cameraEnt.get<Camera>();
	cameraEnt.add<Controller>(camera.position, camera.rotation);
	if (cameraEnt.has<btRigidBody>())
		cameraEnt.get<Controller>().body = &cameraEnt.get<btRigidBody>();
	game.entities.get_system<ModuleSystem>().call($id(INIT), &game);
}

int main(int argc, char* argv[])
{
	Game game;
	Resources& resources = game.resources;
	resources.addPath("../data/");
	game.engine.init(resources.findPath("settings.json"));
	if (Engine::settings["moddir"].is_string())
		resources.addPath(Engine::settings["moddir"].string_value());

	SceneLoader scene(game.entities);

	char scenePath[128] = "testscene.json";
	if (argc == 2)
		strcpy(scenePath, argv[1]);
	else if (Engine::settings["scene"].is_string())
		strcpy(scenePath, Engine::settings["scene"].string_value().c_str());
	init(game, scene, scenePath);

	bool running = true;
	bool active = false;
	bool reload = false;
	bool autoReloadModules = true;
	bool devtools = Engine::settings["devtools"].bool_value();
	SDL_Event e;
	while (running) {
		RenderSystem& renderer = game.entities.get_system<RenderSystem>();
		PhysicsSystem& physics = game.entities.get_system<PhysicsSystem>();
		AudioSystem& audio = game.entities.get_system<AudioSystem>();
		ModuleSystem& modules = game.entities.get_system<ModuleSystem>();
		ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
		Entity cameraEnt = game.entities.get_entity_by_tag("camera");
		Controller& controller = cameraEnt.get<Controller>();
		Camera& camera = cameraEnt.get<Camera>();

		imgui.newFrame();

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
				break;
			}

			if (imgui.processEvent(&e))
				continue;

			if (e.type == SDL_KEYUP) {
				SDL_Keysym keysym = e.key.keysym;

				if (keysym.sym == SDLK_ESCAPE) {
					if (active) {
						active = false;
						game.engine.grabMouse(false);
					} //else running = false;
				}

				if ((keysym.mod == KMOD_LALT || keysym.mod == KMOD_RALT) && keysym.sym == SDLK_RETURN) {
					game.engine.fullscreen(!game.engine.fullscreen());
					renderer.device().resizeRenderTargets();
					continue;
				}
				else if (keysym.mod == KMOD_LCTRL && keysym.sym == SDLK_r) {
					reload = true;
					continue;
				}
				else if (keysym.sym == SDLK_F2) {
					renderer.toggleWireframe();
					continue;
				}
				else if (keysym.sym == SDLK_F3) {
					game.engine.vsync(!game.engine.vsync());
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
				else if (keysym.sym == SDLK_F12) {
					devtools = !devtools;
					continue;
				}
			}

			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
				active = !active;
				game.engine.grabMouse(active);
			}
			else if (e.type == SDL_MOUSEMOTION && active) {
				controller.angles.x += -0.05f * e.motion.yrel;
				controller.angles.y += -0.05f * e.motion.xrel;
			}

			modules.call($id(INPUT), &e);
		}

		controller.enabled = !imgui.usingKeyboard();
		controller.update(game.engine.dt);

		// Modules
		START_MEASURE(moduleTimeMs)
		modules.call($id(UPDATE), &game);
		END_MEASURE(moduleTimeMs)

		// Physics
		START_MEASURE(physTimeMs)
		physics.step(game.entities, game.engine.dt);
		END_MEASURE(physTimeMs)

		if (cameraEnt.has<btRigidBody>()) {
			btRigidBody& body = cameraEnt.get<btRigidBody>();
			camera.position = convert(body.getCenterOfMassPosition());
			if (cameraEnt.has<GroundTracker>())
				controller.onGround = cameraEnt.get<GroundTracker>().onGround;
		} else {
			camera.position = controller.position;
		}
		camera.rotation = controller.rotation;

		// Audio
		START_MEASURE(audioTimeMs)
		audio.update(game.entities, camera);
		END_MEASURE(audioTimeMs)

		// Graphics
		START_MEASURE(renderTimeMs)
		renderer.render(game.entities, camera);
		END_MEASURE(renderTimeMs)

		if (devtools && ImGui::Begin("Debug")) {
			ImGui::Text("Right mouse button to toggle mouse grab.");
			ImGui::Text("FPS: %d (%.3fms)", int(1.0 / game.engine.dt), game.engine.dt * 1000.f);
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
				modules.call($id(settings), $id(DRAW_SETTINGS_MENU), &game);
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
				ImGui::SameLine();
				ImGui::Checkbox("Auto Reload##Modules", &autoReloadModules);
				ImGui::Text("Active modules:");
				for (auto& it : modules.modules) {
					ImGui::Checkbox(it.second.name.c_str(), &it.second.enabled);
					ImGui::SameLine();
					if (ImGui::Button(("Reload##" + it.second.name).c_str())) {
						modules.reload(it.first);
						break; // Must break as iterator will be invalidated
					}
				}
			}
			if (ImGui::CollapsingHeader("Scene")) {
				ImGui::InputText("##Scene Path", scenePath, sizeof(scenePath));
				ImGui::SameLine();
				if (ImGui::Button("Load##ScenePath"))
					reload = true;

				// TODO: Don't do every frame
				std::vector<string> fileList = resources.listFiles(".", ".json");
				const char* sceneFiles[fileList.size()];
				int temp = 0;
				for (auto& it : fileList)
					sceneFiles[temp++] = it.c_str();
				static int selectedScene = 0;
				ImGui::Combo("##Scenes", &selectedScene, sceneFiles, fileList.size());
				ImGui::SameLine();
				if (ImGui::Button("Load##SceneCombo")) {
					strcpy(scenePath, sceneFiles[selectedScene]);
					reload = true;
				}

				const char* prefabs[scene.prefabs.size()];
				temp = 0;
				for (auto it : scene.prefabs)
					prefabs[temp++] = it.first.c_str();
				static int selectedPrefab = 0;
				ImGui::Combo("##Prefabs", &selectedPrefab, prefabs, scene.prefabs.size());
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
					if (e.has<Transform>()) {
						Transform& trans = e.get<Transform>();
						trans.position = pos;
						trans.rotation = camera.rotation;
					}
					if (e.has<btRigidBody>()) {
						btRigidBody& body = e.get<btRigidBody>();
						btTransform trans(convert(camera.rotation), convert(pos));
						body.setWorldTransform(trans);
						body.setLinearVelocity(convert(vel));
					}
				}
			}
		}
		if (devtools)
			ImGui::End();
		//ImGui::ShowTestWindow();
		//ImGui::ShowStyleEditor();
		//ImGui::ShowMetricsWindow();

		ImGui::Render();

		game.engine.swap();

		game.entities.update();

		if (autoReloadModules)
			modules.autoReload();

		if (reload) {
			modules.call($id(DEINIT), &game);
			renderer.reset(game.entities);
			physics.reset();
			scene.reset();
			resources.reset();
			init(game, scene, scenePath);
			reload = false;
		}
	}

	game.entities.get_system<RenderSystem>().reset(game.entities); // TODO: Should not be needed...

	game.entities.remove_system<ImGuiSystem>();
	game.entities.remove_system<ModuleSystem>();
	game.entities.remove_system<AudioSystem>();
	game.entities.remove_system<PhysicsSystem>();
	game.entities.remove_system<RenderSystem>();

	game.engine.deinit();

	return EXIT_SUCCESS;
}

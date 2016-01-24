#include "common.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "glrenderer/renderdevice.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "gui.hpp"
#include "module.hpp"
#include "../game.hpp"
#include "../controller.hpp"

static bool s_autoReloadModules = true;

static void reloadShaders(Game& game)
{
	game.resources.clearTextCache();
	RenderSystem& renderer = game.entities.get_system<RenderSystem>();
	renderer.device().loadShaders();
	renderer.device().setEnvironment(&renderer.env());
	game.entities.for_each<Model>([&](Entity, Model& model) {
		for (auto& material : model.materials)
			material->shaderId[TECH_COLOR] = -1;
	});
}

EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			game.engine.moduleInit();
			ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
			imgui.applyInternalState();
			break;
		}
		case $id(RELOAD_SHADERS):
		{
			reloadShaders(game);
		}
		case $id(DRAW_DEVTOOLS):
		{
			static bool imguiMetrics = false;
			if (ImGui::Begin("Debug")) {
				RenderSystem& renderer = game.entities.get_system<RenderSystem>();
				PhysicsSystem& physics = game.entities.get_system<PhysicsSystem>();
				AudioSystem& audio = game.entities.get_system<AudioSystem>();
				ModuleSystem& modules = game.entities.get_system<ModuleSystem>();
				Entity cameraEnt = game.entities.get_entity_by_tag("camera");
				Controller& controller = cameraEnt.get<Controller>();
				Camera& camera = cameraEnt.get<Camera>();

				ImGui::Text("Right mouse button to toggle mouse grab.");
				ImGui::Text("FPS: %d (%.3fms)", int(1.0 / game.engine.dt), game.engine.dt * 1000.f);
				if (ImGui::CollapsingHeader("Stats")) {
					const RenderDevice::Stats& stats = renderer.device().stats;
					/*ImGui::Text("Physics:      %.3fms", physTimeMs);
					ImGui::Text("Animation:    %.3fms", animTimeMs);
					ImGui::Text("Audio:        %.3fms", audioTimeMs);
					ImGui::Text("Module upd:   %.3fms", moduleTimeMs);
					ImGui::Text("CPU Render:   %.3fms", renderTimeMs);*/
					if (ImGui::TreeNode("Render times")) {
						ImGui::Text("Prerender:    %.3fms", stats.times.prerender);
						ImGui::Text("Upload:       %.3fms", stats.times.upload);
						ImGui::Text("Shadow:       %.3fms", stats.times.shadow);
						ImGui::Text("Reflection:   %.3fms", stats.times.reflection);
						ImGui::Text("Scene:        %.3fms", stats.times.scene);
						ImGui::Text("Postprocess:  %.3fms", stats.times.postprocess);
						ImGui::TreePop();
					}
					ImGui::Text("Lights:       %d", stats.lights);
					ImGui::Text("Triangles:    %d", stats.triangles);
					ImGui::Text("Programs:     %d", stats.programs);
					ImGui::Text("Draw calls:   %d", stats.drawCalls);
					ImGui::Separator();
					ImGui::Text("Voices:       %d/%d (%d)",
						audio.soloud->getActiveVoiceCount(),
						audio.soloud->getMaxActiveVoiceCount(),
						audio.soloud->getVoiceCount());
					ImGui::Separator();
					ImGui::Checkbox("ImGui Metrics", &imguiMetrics);
				}
				if (ImGui::CollapsingHeader("Camera")) {
					ImGui::Text("Position: %.1f %.1f %.1f", camera.position.x, camera.position.y, camera.position.z);
					if (ImGui::Checkbox("Fly", &controller.fly)) {
						if (cameraEnt.has<btRigidBody>()) {
							btRigidBody& body = cameraEnt.get<btRigidBody>();
							body.setGravity(controller.fly ? btVector3(0, 0, 0) : physics.dynamicsWorld->getGravity());
						}
					}
					ImGui::SameLine();
					ImGui::Checkbox("Enable controller", &controller.enabled);
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
					ImGui::SliderFloat("Shadow Darkness", &env.shadowDarkness, 0.0f, 1.0f);
					ImGui::ColorEdit3("Ambient", (float*)&env.ambient);
					ImGui::ColorEdit3("Sun Color", (float*)&env.sunColor);
					ImGui::SliderFloat3("Sun Pos", (float*)&env.sunPosition, -15.f, 15.f);
					ImGui::ColorEdit3("Fog Color", (float*)&env.fogColor);
					ImGui::SliderFloat("Fog Density", &env.fogDensity, 0.0f, 1.0f);
					ImGui::SliderInt("Sky Type", (int*)&env.skyType, 0, Environment::SKY_COUNT-1);
				}
				if (ImGui::CollapsingHeader("Post Effects")) {
					Environment& env = renderer.env();
					ImGui::SliderFloat3("Vignette", (float*)&env.vignette, 0.0f, 1.0f);
					ImGui::SliderFloat("Sepia", &env.sepia, 0.0f, 1.0f);
					ImGui::SliderFloat("Saturation", &env.saturation, -1.0f, 1.0f);
					ImGui::SliderFloat("Chromatic Aberration", &env.chromaticAberration, 0.0f, 1.0f);
					ImGui::SliderFloat("Scanlines", &env.scanlines, 0.0f, 4.0f);
				}
				if (ImGui::CollapsingHeader("Modules")) {
					if (ImGui::Button("Reload all##Modules")) {
						modules.load(Engine::settings["modules"]);
						modules.call($id(INIT), &game);
					}
					ImGui::SameLine();
					ImGui::Checkbox("Auto Reload##Modules", &s_autoReloadModules);
					ImGui::Text("Active modules:");
					for (auto& it : modules.modules) {
						ImGui::Checkbox(it.second.name.c_str(), &it.second.enabled);
						ImGui::SameLine();
						if (ImGui::Button(("Reload##" + it.second.name).c_str())) {
							modules.reload(it.first);
							break; // Must break as iterator will be invalidated
						}
						ImGui::SameLine();
						if (ImGui::Button(("Init##" + it.second.name).c_str())) {
							modules.call(it.first, $id(INIT), &game);
						}
					}
				}
				if (ImGui::CollapsingHeader("Scene")) {
					if (ImGui::Button("Reload Shaders"))
						reloadShaders(game);
					static char tempScenePath[128] = {0};
					strncpy(tempScenePath, game.scenePath.c_str(), sizeof(tempScenePath)-1);
					ImGui::InputText("##Scene Path", tempScenePath, sizeof(tempScenePath));
					game.scenePath = tempScenePath;
					ImGui::SameLine();
					if (ImGui::Button("Load##ScenePath"))
						game.reload = true;

					// TODO: Don't do every frame
					std::vector<string> fileList = game.resources.listFiles(".", ".json");
					const char* sceneFiles[fileList.size()];
					int temp = 0;
					for (auto& it : fileList)
						sceneFiles[temp++] = it.c_str();
					static int selectedScene = 0;
					ImGui::Combo("##Scenes", &selectedScene, sceneFiles, fileList.size());
					ImGui::SameLine();
					if (ImGui::Button("Load##SceneCombo")) {
						game.scenePath = sceneFiles[selectedScene];
						game.reload = true;
					}

					const char* prefabs[game.scene.prefabs.size()];
					temp = 0;
					for (auto it : game.scene.prefabs)
						prefabs[temp++] = it.first.c_str();
					static int selectedPrefab = 0;
					ImGui::Combo("##Prefabs", &selectedPrefab, prefabs, game.scene.prefabs.size());
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
						Entity e = game.scene.instantiate(game.scene.prefabs[prefabs[selectedPrefab]], game.resources);
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
				if (ImGui::CollapsingHeader("Entities")) {
					game.entities.for_each<Transform>([](Entity e, Transform& trans) {
						string label = "Entity #" + std::to_string(e.get_index());
						if (ImGui::SliderFloat3(label.c_str(), &trans.position[0], -100, 100))
							trans.dirty = true;
						if (ImGui::SliderFloat3(("Scale##" + label).c_str(), &trans.scale[0], 0, 10))
							trans.dirty = true;
						if (ImGui::SliderFloat4(("Rot##" + label).c_str(), &trans.rotation[0], -1, 1))
							trans.dirty = true;
						ImGui::Separator();
					});
				}
			}
			ImGui::End();

			if (imguiMetrics)
				ImGui::ShowMetricsWindow();

			break;
		}
		case $id(FRAME_END):
		{
			if (s_autoReloadModules)
				game.entities.get_system<ModuleSystem>().autoReload();
		}
	}
}

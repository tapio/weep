#include "common.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "../controller.hpp"
#include "../game.hpp"
#include "imgui/imgui.h"

static const int MinimalWindow =
	ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|
	ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings;

static ImFont* font = 0;
static btTransform startPos;
static vec3 goalPos = vec3(INFINITY, INFINITY, INFINITY);
static float gameTime = 0;
static bool levelStarted = false;
static bool levelComplete = false;

void setPos(Entity e, vec3 pos) {
	// TODO: Need to make this position setting easier on engine level
	if (e.has<Model>()) {
		Model& model = e.get<Model>();
		model.position = pos;
	}
	if (e.has<btRigidBody>()) {
		btRigidBody& body = e.get<btRigidBody>();
		btTransform trans(body.getCenterOfMassTransform().getRotation(), convert(pos));
		body.setWorldTransform(trans);
	}
}

void generatePole(const Json& block, vec3 pos, SceneLoader& loader, Resources& resources) {
	for (int i = 0; i < 10; ++i) {
		pos.y -= 1.f;
		Entity e = loader.instantiate(block, resources);
		setPos(e, pos);
	}
}


EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			ImGui::SetInternalState(game.imgui);
			ImGuiIO& io = ImGui::GetIO();
			// TODO: Super fragile, figure out a way to get the font
			if (io.Fonts->Fonts.size() <= 1) {
				string fontPath = game.resources.findPath("fonts/Orbitron-Black.ttf");
				font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 48.0f, NULL, io.Fonts->GetGlyphRangesDefault());
			} else font = io.Fonts->Fonts.back();
			gameTime = 0;
			Entity pl = game.entities.get_entity_by_tag("camera");
			if (!pl.is_alive() || !pl.has<btRigidBody>())
				return;

			btRigidBody& body = pl.get<btRigidBody>();
			startPos = body.getCenterOfMassTransform();
			break;
		}
		case $id(UPDATE):
		{
			if (!levelStarted)
				return;
			Entity pl = game.entities.get_entity_by_tag("camera");
			if (!pl.is_alive() || !pl.has<btRigidBody>())
				return;

			if (!levelComplete)
				gameTime += game.dt;

			btRigidBody& body = pl.get<btRigidBody>();
			vec3 curPos = convert(body.getCenterOfMassPosition());
			if (curPos.y < -3) {
				body.setCenterOfMassTransform(startPos);
				gameTime = 0;
				levelComplete = false;
			}

			ASSERT(font);
			ImGui::PushFont(font);
			if (glm::distance2(curPos, goalPos) < 1.1f || levelComplete) {
				levelComplete = true;
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("", NULL, MinimalWindow);
				ImGui::Text("Good Job! %.2f s", gameTime);
				ImGui::End();
			} else if (gameTime < 3) {
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("", NULL, MinimalWindow);
				ImGui::Text("Run to the end!");
				ImGui::End();
			} else {
				ImGui::SetNextWindowPos(ImVec2(20, 20));
				ImGui::Begin("", NULL, MinimalWindow);
				ImGui::Text("Time: %.2f", gameTime);
				ImGui::End();
			}
			ImGui::PopFont();

			break;
		}
		case $id(GENERATE_LEVEL):
		{
			std::srand(std::time(0));
			SceneLoader loader(game.entities);
			loader.load("skyrunner.json", game.resources);
			const Json& block = loader.prefabs["skyblock"];
			const Json& box = loader.prefabs["skybox"];
			vec3 pos(0, -1, 0);
			for (int i = 0; i < 100; i++) {
				Entity e = loader.instantiate(block, game.resources);
				setPos(e, pos);
				// Adjust position
				float xrand = glm::linearRand(-1.f, 1.f);
				if (xrand < -0.6f || xrand > 0.6f)
					pos.x += xrand;
				else pos.x = 0;
				if (glm::linearRand(0.f, 1.f) < 0.25f)
					pos.y += glm::linearRand(-0.5f, 1.2f);
				if (glm::linearRand(0.f, 1.f) < 0.2f) {
					pos.z -= glm::linearRand(1.5f, 3.0f);
					generatePole(block, pos, loader, game.resources);
				} else pos.z -= 1.f;

				if (glm::linearRand(0.f, 1.f) < 0.35f) {
					Entity ebox = loader.instantiate(box, game.resources);
					vec3 offset(glm::linearRand(-0.4, 0.4), 0.8, glm::linearRand(-0.4, 0.4));
					setPos(ebox, pos + offset);
				}
			}
			Entity e = loader.instantiate(loader.prefabs["goalblock"], game.resources);
			setPos(e, pos);
			goalPos = pos + vec3(0, 1, 0);

			Entity cameraEnt = game.entities.get_entity_by_tag("camera");
			Camera& camera = cameraEnt.get<Camera>();
			cameraEnt.add<Controller>(camera.position, camera.rotation);
			Controller& controller = cameraEnt.get<Controller>();
			if (cameraEnt.has<btRigidBody>())
				controller.body = &cameraEnt.get<btRigidBody>();
			levelStarted = true;
			break;
		}
	}
}


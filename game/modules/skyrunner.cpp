#include "common.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "gui.hpp"
#include "../controller.hpp"
#include "../game.hpp"

static const int MinimalWindow =
	ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|
	ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings;

static vec3 startPos = vec3(0, 1, 0);
static vec3 goalPos = vec3(INFINITY, INFINITY, INFINITY);
static float gameTime = 0;
static float waitTime = 0;
static bool levelStarted = false;
static bool levelComplete = false;
static int level = 1;


void setPos(Entity e, vec3 pos) {
	// TODO: Need to make this position setting easier on engine level
	if (e.has<Transform>())
		e.get<Transform>().position = pos;
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

void generateLevel1(Game& game, vec3 start);
void generateLevel2(Game& game, vec3 start);
void generateLevel3(Game& game, vec3 start);



EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
			imgui.applyInternalState();
			//imgui.loadFont("skyrunner_big", game.resources.findPath("fonts/Orbitron-Black.ttf"), 48.f);
			gameTime = 0;
			waitTime = 0;
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
			else waitTime += game.dt;

			btRigidBody& body = pl.get<btRigidBody>();
			vec3 curPos = convert(body.getCenterOfMassPosition());
			if (curPos.y < -3) {
				btTransform trans(convert(quat()), convert(startPos));
				body.setCenterOfMassTransform(trans);
				gameTime = 0;
				waitTime = 0;
				levelComplete = false;
			}

			ImFont* font = game.entities.get_system<ImGuiSystem>().getFont("skyrunner_big");
			ASSERT(font);
			ImGui::PushFont(font);
			if (glm::distance2(curPos, goalPos) < 1.1f || levelComplete) {
				levelComplete = true;
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("", NULL, MinimalWindow);
				ImGui::Text("Good Job! %.2f s", gameTime);
				ImGui::End();
				if (waitTime >= 3) {
					gameTime = 0;
					waitTime = 0;
					levelComplete = false;
					level++;
					startPos = curPos + vec3(0, 0, -1.5);
					if (level == 2) generateLevel2(game, startPos);
					if (level == 3) generateLevel3(game, startPos);
				}
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
			generateLevel1(game, startPos);

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

void generateLevel1(Game& game, vec3 pos)
{
	SceneLoader loader(game.entities);
	loader.load("skyrunner.json", game.resources);
	const Json& block = loader.prefabs["skyblock1"];
	const Json& box = loader.prefabs["skybox"];
	pos.y -= 1;
	for (int i = 0; i < 60; i++) {
		Entity e = loader.instantiate(block, game.resources);
		setPos(e, pos);
		// Adjust position
		if (glm::linearRand(0.f, 1.f) < 0.25f)
			pos.x += glm::linearRand(-0.6f, 0.6f);
		if (glm::linearRand(0.f, 1.f) < 0.20f)
			pos.y += glm::linearRand(-0.5f, 0.5f);
		if (glm::linearRand(0.f, 1.f) < 0.25f) {
			pos.z -= glm::linearRand(2.0f, 3.5f);
			generatePole(block, pos, loader, game.resources);
		} else pos.z -= 1.f;

		if (glm::linearRand(0.f, 1.f) < 0.20f) {
			Entity ebox = loader.instantiate(box, game.resources);
			vec3 offset(glm::linearRand(-0.4, 0.4), 0.8, glm::linearRand(-0.4, 0.4));
			setPos(ebox, pos + offset);
		}
	}
	Entity e = loader.instantiate(loader.prefabs["goalblock"], game.resources);
	setPos(e, pos);
	goalPos = pos + vec3(0, 1, 0);
}

void generateLevel2(Game& game, vec3 pos)
{
	SceneLoader loader(game.entities);
	loader.load("skyrunner.json", game.resources);
	const Json& block = loader.prefabs["skyblock2"];
	const Json& box = loader.prefabs["skybox"];
	pos.y -= 1;
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
}

void generateLevel3(Game& game, vec3 pos)
{
	SceneLoader loader(game.entities);
	loader.load("skyrunner.json", game.resources);
	const Json& block = loader.prefabs["skyblock3"];
	const Json& box = loader.prefabs["skybox"];
	pos.y -= 1;
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
}

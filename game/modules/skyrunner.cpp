#include "common.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "components.hpp"
#include "audio.hpp"
#include "gui.hpp"
#include "../controller.hpp"
#include "../game.hpp"
#include "SDL2/SDL_events.h"

static vec3 startPos = vec3(0, 1, 0);
static vec3 goalPos = vec3(INFINITY, INFINITY, INFINITY);
static float gameTime = 0;
static float waitTime = 0;
static bool levelStarted = false;
static bool levelComplete = false;
static int level = 1;
static enum {
	MENU_HIDDEN,
	MENU_MAIN,
	MENU_PAUSE,
	MENU_HELP,
	MENU_OPTIONS
} menuState = MENU_MAIN;

static void generatePole(const Json& block, vec3 pos, SceneLoader& loader, Resources& resources) {
	for (int i = 0; i < 10; ++i) {
		pos.y -= 1.f;
		Entity e = loader.instantiate(block, resources);
		e.get<Transform>().setPosition(pos);
	}
}

static void generateLevel1(Game& game, vec3 start);
static void generateLevel2(Game& game, vec3 start);
static void generateLevel3(Game& game, vec3 start);


static void startNextLevel(Game& game)
{
	std::srand(std::time(0));
	// TODO: Simplify
	Entity cameraEnt = game.entities.get_entity_by_tag("camera");
	cameraEnt.kill();
	game.entities.update();
	ASSERT(!game.entities.has_tagged_entity("camera"));
	if (level <= 1) generateLevel1(game, startPos);
	else if (level == 2) generateLevel2(game, startPos);
	else if (level == 3) generateLevel3(game, startPos);

	ASSERT(game.entities.has_tagged_entity("camera"));
	cameraEnt = game.entities.get_entity_by_tag("camera");
	ASSERT(cameraEnt.is_alive());
	cameraEnt.get<Transform>().setPosition(startPos);
	Camera& camera = cameraEnt.get<Camera>();
	cameraEnt.add<Controller>(camera.position, camera.rotation);
	Controller& controller = cameraEnt.get<Controller>();
	controller.body = &cameraEnt.get<btRigidBody>();
	controller.fast = 1.f;

	levelStarted = true;
}

static void doMainMenu(Game& game)
{
	ImGui::SetNextWindowPosCenter();
	ImGui::Begin("##SkyrunnerMenu", NULL, ImGuiSystem::MinimalWindow);
	{
		ScopedFont sf(game.entities, $id(skyrunner_big));
		ImGui::Text("SkyRunner");
	}{
		ScopedFont sf(game.entities, $id(skyrunner_menu));
		ImVec2 bsize(300.f, 0.f);
		if (menuState == MENU_MAIN || menuState == MENU_PAUSE) {
			if (ImGui::Button(menuState == MENU_MAIN ? "New Game" : "Continue", bsize)) {
				if (menuState == MENU_MAIN)
					startNextLevel(game);
				menuState = MENU_HIDDEN;
			}
			if (ImGui::Button("Instructions", bsize))
				menuState = MENU_HELP;
			if (ImGui::Button("Options", bsize))
				menuState = MENU_OPTIONS;
			if (ImGui::Button("Exit", bsize))
				std::exit(0);
		} else if (menuState == MENU_HELP) {
			ImGui::Text("Run to the end as\nfast as you can.");
			ImGui::Text("Look: mouse");
			ImGui::Text("Move: arrow keys");
			ImGui::Text("Jump: space");
			if (ImGui::Button("Back", bsize))
				menuState = MENU_MAIN;
		} else if (menuState == MENU_OPTIONS) {
			game.entities.get_system<ModuleSystem>().call("settings", $id(DRAW_SETTINGS_MENU), &game);
			if (ImGui::Button("Back", bsize))
				menuState = MENU_MAIN;
		}
	}
	ImGui::End();
}


EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			ImGuiSystem& imgui = game.entities.get_system<ImGuiSystem>();
			imgui.applyInternalState();
			gameTime = 0;
			waitTime = 0;
			break;
		}
		case $id(INPUT):
		{
			const SDL_Event& e = *static_cast<SDL_Event*>(param);
			if (e.type == SDL_KEYUP) {
				SDL_Keysym keysym = e.key.keysym;

				if (keysym.sym == SDLK_ESCAPE) {
					if (menuState == MENU_HIDDEN)
						menuState = MENU_PAUSE;
					else if (menuState == MENU_PAUSE)
						menuState = MENU_HIDDEN;
				}
			}
			break;
		}
		case $id(UPDATE):
		{
			if (menuState != MENU_HIDDEN) {
				doMainMenu(game);
				return;
			}
			Entity pl = game.entities.get_entity_by_tag("camera");
			if (!pl.is_alive() || !pl.has<btRigidBody>())
				return;

			if (!levelComplete)
				gameTime += game.engine.dt;
			else waitTime += game.engine.dt;

			Transform& transform = pl.get<Transform>();
			vec3 curPos = transform.position;
			if (curPos.y < -3) {
				transform.rotation = quat();
				transform.setPosition(startPos);
				gameTime = 0;
				waitTime = 0;
				levelComplete = false;
			}

			ScopedFont sf(game.entities, $id(skyrunner_big));
			if (glm::distance2(curPos, goalPos) < 1.1f || levelComplete) {
				levelComplete = true;
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("", NULL, ImGuiSystem::MinimalWindow);
				ImGui::Text("Good Job! %.2f s", gameTime);
				ImGui::End();
				if (waitTime >= 3) {
					gameTime = 0;
					waitTime = 0;
					levelComplete = false;
					level++;
					startPos = curPos + vec3(0, 0, -1.5);
					if (level <= 3)
						startNextLevel(game);
				}
			} else if (gameTime < 3) {
				ImGui::SetNextWindowPosCenter();
				ImGui::Begin("", NULL, ImGuiSystem::MinimalWindow);
				ImGui::Text("Run to the end!");
				ImGui::End();
			} else {
				ImGui::SetNextWindowPos(ImVec2(20, 20));
				ImGui::Begin("", NULL, ImGuiSystem::MinimalWindow);
				ImGui::Text("Time: %.2f", gameTime);
				ImGui::End();
			}

			break;
		}
	}
}

static void generateLevel1(Game& game, vec3 pos)
{
	SceneLoader loader(game.entities);
	loader.load("skyrunner.json", game.resources);
	const Json& block = loader.prefabs["skyblock1"];
	const Json& box = loader.prefabs["skybox"];
	pos.y -= 1;
	for (int i = 0; i < 60; i++) {
		Entity e = loader.instantiate(block, game.resources);
		e.get<Transform>().setPosition(pos);
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
			ebox.get<Transform>().setPosition(pos + offset);
		}
	}
	Entity e = loader.instantiate(loader.prefabs["goalblock"], game.resources);
	e.get<Transform>().setPosition(pos);
	goalPos = pos + vec3(0, 1, 0);
}

static void generateLevel2(Game& game, vec3 pos)
{
	SceneLoader loader(game.entities);
	loader.load("skyrunner.json", game.resources);
	const Json& block = loader.prefabs["skyblock2"];
	const Json& box = loader.prefabs["skybox"];
	pos.y -= 1;
	for (int i = 0; i < 100; i++) {
		Entity e = loader.instantiate(block, game.resources);
		e.get<Transform>().setPosition(pos);
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
			ebox.get<Transform>().setPosition(pos + offset);
		}
	}
	Entity e = loader.instantiate(loader.prefabs["goalblock"], game.resources);
	e.get<Transform>().setPosition(pos);
	goalPos = pos + vec3(0, 1, 0);
}

static void generateLevel3(Game& game, vec3 pos)
{
	SceneLoader loader(game.entities);
	loader.load("skyrunner.json", game.resources);
	const Json& block = loader.prefabs["skyblock3"];
	const Json& box = loader.prefabs["skybox"];
	pos.y -= 1;
	for (int i = 0; i < 100; i++) {
		Entity e = loader.instantiate(block, game.resources);
		e.get<Transform>().setPosition(pos);
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
			ebox.get<Transform>().setPosition(pos + offset);
		}
	}
	Entity e = loader.instantiate(loader.prefabs["goalblock"], game.resources);
	e.get<Transform>().setPosition(pos);
	goalPos = pos + vec3(0, 1, 0);
}

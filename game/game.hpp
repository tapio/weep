#pragma once
#include "common.hpp"
#include "engine.hpp"
#include "resources.hpp"
#include "module.hpp"
#include "scene.hpp"
#include "gui.hpp"
#include <ecs/ecs.hpp>

using namespace ecs;

struct Game {
	Entities& entities;
	Engine engine = {};
	Resources resources = {};
	SceneLoader scene = {};
	string scenePath = "testscene.json";
	bool reload = false;
	bool restoreCam = false; // Must be initially false, set to true with "reload" when desired

	void moduleInit() {
		engine.moduleInit();
		#ifdef MODULE_NAME
		ECS::worlds = &entities; // TODO
		#endif
		ImGuiSystem& imgui = entities.get_system<ImGuiSystem>();
		imgui.applyInternalState();
	}
};

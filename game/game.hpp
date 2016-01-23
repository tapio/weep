#pragma once
#include "common.hpp"
#include "engine.hpp"
#include "resources.hpp"
#include "module.hpp"
#include "scene.hpp"
#include <id/id.hpp>

struct Game {
	Engine engine = {};
	Entities entities = {};
	Resources resources = {};
	SceneLoader scene = {};
	string scenePath = "testscene.json";
	bool reload = false;
};

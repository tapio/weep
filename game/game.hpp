#pragma once
#include "common.hpp"
#include "engine.hpp"
#include "resources.hpp"
#include "module.hpp"
#include "scene.hpp"

struct Game {
	Entities& entities;
	Engine engine = {};
	Resources resources = {};
	SceneLoader scene = {};
	string scenePath = "testscene.json";
	bool reload = false;
	bool restoreCam = false; // Must be initially false, set to true with "reload" when desired
};

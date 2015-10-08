#pragma once
#include "common.hpp"
#include "engine.hpp"
#include "resources.hpp"
#include "module.hpp"

struct Game {
	Engine engine = {};
	Entities entities = {};
	Resources resources = {};
	Modules modules = {};
};

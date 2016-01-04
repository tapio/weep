#pragma once
#include "common.hpp"
#include "engine.hpp"
#include "resources.hpp"
#include "module.hpp"
#include <id/id.hpp>

struct Game {
	Engine engine = {};
	Entities entities = {};
	Resources resources = {};
};

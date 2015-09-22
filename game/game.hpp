#pragma once
#include "common.hpp"
#include "resources.hpp"
#include "module.hpp"

struct Game {
	Entities entities = {};
	Resources resources = {};
	Modules modules = {};
};

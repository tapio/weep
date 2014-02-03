#pragma once
#include "common.hpp"

struct Geometry;
struct Material;

struct Model
{
	mat4 transform = mat4();
	Geometry* geometry = nullptr;
	std::shared_ptr<Material> material;
};

#pragma once
#include "common.hpp"

struct Geometry;
struct Material;

struct Model
{
	mat4 transform = mat4();
	std::shared_ptr<Geometry> geometry;
	std::shared_ptr<Material> material;
};

#pragma once
#include "common.hpp"

struct Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
};

struct Geometry
{
	Geometry() {}
	Geometry(const string& path);
	~Geometry();

	bool upload();

	std::vector<Vertex> vertices;

	uint vao = 0;
	uint vbo = 0;
};

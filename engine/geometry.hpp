#pragma once
#include "common.hpp"

struct Image;

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
	Geometry(const Image& heightmap);
	~Geometry();

	bool upload();

	std::vector<Vertex> vertices;
	std::vector<uint> indices;

	uint vao = 0;
	uint vbo = 0;
	uint ebo = 0;
};

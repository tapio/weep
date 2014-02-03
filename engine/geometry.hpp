#pragma once
#include "common.hpp"

struct Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
};

struct Triangle
{
	unsigned vertex[3];
};

struct Geometry
{
	Geometry() {}
	Geometry(const string& path);
	~Geometry();

	bool upload();

	std::vector<Vertex> vertices;
	//std::vector<Triangle> faces;
	uint vao = 0;
	uint vbo = 0;

	static Geometry createPlane(float width, float height);
	static Geometry createCube(float size);
};

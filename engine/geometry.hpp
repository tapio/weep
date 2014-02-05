#pragma once
#include "common.hpp"

struct Image;

struct Vertex
{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
};

struct BoundingBox
{
	vec3 min = vec3(INFINITY, INFINITY, INFINITY);
	vec3 max = vec3(INFINITY, INFINITY, INFINITY);
};

struct Geometry
{
	Geometry() {}
	Geometry(const string& path);
	Geometry(const Image& heightmap);
	~Geometry();

	void calculateBoundingSphere();
	void calculateBoundingBox();
	void calculateNormals();
	void normalizeNormals();

	std::vector<Vertex> vertices;
	std::vector<uint> indices;

	BoundingBox boundingBox;
	float boundingRadius;

	uint vao = 0;
	uint vbo = 0;
	uint ebo = 0;
};

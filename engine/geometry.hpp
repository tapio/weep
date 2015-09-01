#pragma once
#include "common.hpp"

struct Image;

enum GeometryAttributeIndex {
	ATTR_POSITION = 0,
	ATTR_TEXCOORD,
	ATTR_NORMAL,
	ATTR_COLOR,
	ATTR_MAX
};

struct Bounds
{
	vec3 min = vec3(INFINITY, INFINITY, INFINITY);
	vec3 max = vec3(INFINITY, INFINITY, INFINITY);
	float radius = INFINITY;
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
	void setupAttributes();

	struct Attribute {
		int components = 0;
		int offset = 0;
	} attributes[ATTR_MAX];
	int vertexSize = 0;
	uint numVertices = 0;

	std::vector<vec3> positions;
	std::vector<vec2> positions2d;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	std::vector<uint> indices;
	std::vector<char> vertexData;

	Bounds bounds;

	int renderId = -1;
};

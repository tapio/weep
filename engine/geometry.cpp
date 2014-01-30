#include "geometry.hpp"

Geometry::~Geometry()
{
	ASSERT(!vao && !vbo);
}

Geometry Geometry::createPlane(float width, float height)
{
	Geometry geom;
	geom.vertices.reserve(6);
	geom.vertices.push_back({ vec3( 0.5f * width, -0.5f * height, 0), vec2(1,1), vec3(0,1,0) });
	geom.vertices.push_back({ vec3(-0.5f * width, -0.5f * height, 0), vec2(0,1), vec3(0,1,0) });
	geom.vertices.push_back({ vec3(-0.5f * width,  0.5f * height, 0), vec2(0,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3( 0.5f * width,  0.5f * height, 0), vec2(1,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3( 0.5f * width, -0.5f * height, 0), vec2(1,1), vec3(0,1,0) });
	geom.vertices.push_back({ vec3(-0.5f * width,  0.5f * height, 0), vec2(0,0), vec3(0,1,0) });
	return geom;
}

Geometry Geometry::createCube(float /*size*/)
{
	Geometry geom;
	geom.vertices.resize(12);
	return geom;
}

#include "geometry.hpp"
#include "platform.hpp"
#include "glutil.hpp"

Geometry::~Geometry()
{
	if (vbo) glDeleteBuffers(1, &vbo);
	if (vao) glDeleteVertexArrays(1, &vao);
}

bool Geometry::upload()
{
	if (vertices.empty()) {
		logError("Cannot upload empty geometry");
		return false;
	}
	glutil::checkGL("Pre geometry upload");

	if (!vbo) glGenBuffers(1, &vbo);
	if (!vao) glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	// TexCoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
	// Normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

	glutil::checkGL("Post geometry upload");
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

Geometry Geometry::createPlane(float width, float height)
{
	Geometry geom;
	geom.vertices.reserve(6);
	geom.vertices.push_back({ vec3( 0.5f * width, 0, -0.5f * height), vec2(0,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3(-0.5f * width, 0, -0.5f * height), vec2(0,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3(-0.5f * width, 0,  0.5f * height), vec2(0,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3( 0.5f * width, 0,  0.5f * height), vec2(0,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3( 0.5f * width, 0, -0.5f * height), vec2(0,0), vec3(0,1,0) });
	geom.vertices.push_back({ vec3( -0.5f * width, 0, 0.5f * height), vec2(0,0), vec3(0,1,0) });
	return geom;
}

Geometry Geometry::createCube(float size)
{
	Geometry geom;
	geom.vertices.resize(12);
	return geom;
}

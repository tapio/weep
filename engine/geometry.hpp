#pragma once
#include "common.hpp"
#include "components.hpp"

struct Image;

// Remember to modify uniforms.glsl layout to match
enum GeometryAttributeIndex {
	ATTR_POSITION = 0,
	ATTR_TEXCOORD,
	ATTR_NORMAL,
	ATTR_TANGENT,
	ATTR_COLOR,
	ATTR_BONE_INDEX,
	ATTR_BONE_WEIGHT,
	ATTR_MAX
};

struct Batch
{
	Batch() {}
	~Batch();

	void setupAttributes();

	struct Attribute {
		int components = 0;
		int type = 0;
		int offset = 0;
		bool normalized = false;
	} attributes[ATTR_MAX];
	int vertexSize = 0;
	uint numVertices = 0;

	std::vector<vec3> positions;
	std::vector<vec2> positions2d;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	std::vector<u8vec4> boneindices;
	std::vector<u8vec4> boneweights;
	std::vector<uint> indices;
	std::vector<char> vertexData;
	uint materialIndex = 0;
	int renderId = -1;
	string name;
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
	void applyMatrix(mat4 transform);
	void generateCollisionTriMesh(bool deduplicateVertices = true);
	void merge(const Geometry& geometry, vec3 offset, int materialIndexOffset = 0);

	std::vector<Batch> batches;

	struct Animation {
		uint start = 0;
		uint length = 0;
		float frameRate = 1.f;
		string name;
	};

	std::vector<mat3x4> bones;
	std::vector<mat3x4> animFrames;
	std::vector<int> boneParents;
	std::vector<Animation> animations;

	Bounds bounds;

	class btTriangleMesh* collisionMesh = nullptr;

private:
	bool loadObj(const string& path);
	bool loadIqm(const string& path);
};


inline mat3x4 multiplyBones(const mat3x4& lhs, const mat3x4& rhs) {
	return mat3x4(
		vec4(rhs[0] * lhs[0].x + rhs[1] * lhs[0].y + rhs[2] * lhs[0].z + vec4(0, 0, 0, lhs[0].w)),
		vec4(rhs[0] * lhs[1].x + rhs[1] * lhs[1].y + rhs[2] * lhs[1].z + vec4(0, 0, 0, lhs[1].w)),
		vec4(rhs[0] * lhs[2].x + rhs[1] * lhs[2].y + rhs[2] * lhs[2].z + vec4(0, 0, 0, lhs[2].w)));
}

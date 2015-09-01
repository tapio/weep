#include "geometry.hpp"
#include "image.hpp"
#include <sstream>
#include <fstream>

namespace {
	std::vector<std::string> split(const std::string &str, char delim) {
		std::stringstream ss(str);
		std::vector<std::string> elems;
		std::string item;
		while (std::getline(ss, item, delim))
			elems.push_back(item);
		return elems;
	}
}

Geometry::Geometry(const string& path)
{
	uint lineNumber = 0;
	std::string row;
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		logError("Failed to load object %s", path.c_str());
		return;
	}
	std::vector<vec3> tmpVerts;
	std::vector<vec2> tmpUvs;
	std::vector<vec3> tmpNormals;
	while (std::getline(file, row)) {
		++lineNumber;
		std::istringstream srow(row);
		float x,y,z;
		std::string tempst;
		if (row.substr(0,2) == "v ") {  // Vertices
			srow >> tempst >> x >> y >> z;
			tmpVerts.push_back(vec3(x, y, z));
		} else if (row.substr(0,2) == "vt") {  // Texture Coordinates
			srow >> tempst >> x >> y;
			tmpUvs.push_back(vec2(x, -y));
		} else if (row.substr(0,2) == "vn") {  // Normals
			srow >> tempst >> x >> y >> z;
			tmpNormals.push_back(glm::normalize(vec3(x, y, z)));
		} else if (row.substr(0,2) == "f ") {  // Faces
			srow >> tempst; // Eat away prefix
			// Parse face point's coordinate references
			for (int i = 1; srow >> tempst; ++i) {
				if (i > 4) {
					logError("Only triangle and quad faces are supported in %s:%d", path.c_str(), lineNumber);
					break;
				}
				std::vector<std::string> indices = split(tempst, '/');
				if (indices.size() == 0 || indices.size() > 3) {
					logError("Invalid face definition in %s:%d", path.c_str(), lineNumber);
					continue;
				} else if (indices.size() < 3) {
					indices.resize(3);
				}
				// Vertex indices are 1-based in the file
				if (i <= 3) {
					if (!indices[0].empty()) positions.push_back(tmpVerts.at(std::stoi(indices[0]) - 1));
					if (!indices[1].empty()) texcoords.push_back(tmpUvs.at(std::stoi(indices[1]) - 1));
					if (!indices[2].empty()) normals.push_back(tmpNormals.at(std::stoi(indices[2]) - 1));
				} else if (i == 4) {
					// Manually create a new triangle
					if (!indices[0].empty()) {
						size_t lastPos = positions.size();
						positions.push_back(tmpVerts.at(std::stoi(indices[0]) - 1));
						positions.push_back(positions.at(lastPos - 3));
						positions.push_back(positions.at(lastPos - 1));
					}
					if (!indices[1].empty()) {
						size_t lastPos = texcoords.size();
						texcoords.push_back(tmpUvs.at(std::stoi(indices[1]) - 1));
						texcoords.push_back(texcoords.at(lastPos - 3));
						texcoords.push_back(texcoords.at(lastPos - 1));
					}
					if (!indices[2].empty()) {
						size_t lastPos = normals.size();
						normals.push_back(tmpNormals.at(std::stoi(indices[2]) - 1));
						normals.push_back(normals.at(lastPos - 3));
						normals.push_back(normals.at(lastPos - 1));
					}
				}
			}
		}
	}
	calculateBoundingSphere();
	calculateBoundingBox();
	logDebug("Loaded mesh %s, vert/uv/n: %d/%d/%d, bound r: %f",
		path.c_str(), positions.size(), texcoords.size(), normals.size(), bounds.radius);
	if (normals.empty())
		calculateNormals();
	else normalizeNormals();
	setupAttributes();
}

Geometry::Geometry(const Image& heightmap)
{
	int wpoints = heightmap.width + 1;
	int numVerts = wpoints * (heightmap.height + 1);
	positions.resize(numVerts);
	texcoords.resize(numVerts);
	normals.resize(numVerts);
	for (int j = 0; j <= heightmap.height; ++j) {
		for (int i = 0; i <= heightmap.width; ++i) {
			// Create the vertex
			float x = i - heightmap.width * 0.5f;
			float z = j - heightmap.height * 0.5f;
			float y = heightmap.data[heightmap.channels * (j * heightmap.width + i)] / 255.0f;
			int vert = j * wpoints + i;
			positions[vert] = vec3(x, y, z);
			texcoords[vert] = vec2((float)i / heightmap.width, (float)j / heightmap.height);
			normals[vert] = vec3(0, 1, 0);
			// Indexed faces
			if (i == heightmap.width || j == heightmap.height)
				continue;
			uint a = i + wpoints * j;
			uint b = i + wpoints * (j + 1);
			uint c = (i + 1) + wpoints * (j + 1);
			uint d = (i + 1) + wpoints * j;
			uint triangles[] = { a, b, d, b, c, d };
			indices.insert(indices.end(), triangles, triangles + 6);
		}
	}
	calculateBoundingSphere();
	calculateBoundingBox();
	calculateNormals();
	setupAttributes();
}

Geometry::~Geometry()
{
	ASSERT(renderId == -1);
}

void Geometry::setupAttributes()
{
	ASSERT(positions.empty() || positions2d.empty());
	int offset = 0;
	const char* dataArrays[ATTR_MAX] = {0};
	if (!positions.empty()) {
		Attribute& attr = attributes[ATTR_POSITION];
		attr.components = 3;
		attr.offset = offset;
		offset += sizeof(positions[0]);
		numVertices = positions.size();
		dataArrays[ATTR_POSITION] = (char*)&positions[0];
	}
	if (!positions2d.empty()) {
		Attribute& attr = attributes[ATTR_POSITION];
		attr.components = 2;
		attr.offset = offset;
		offset += sizeof(positions2d[0]);
		numVertices = positions2d.size();
		dataArrays[ATTR_POSITION] = (char*)&positions2d[0];
	}
	if (!texcoords.empty()) {
		Attribute& attr = attributes[ATTR_TEXCOORD];
		attr.components = 2;
		attr.offset = offset;
		offset += sizeof(texcoords[0]);
		dataArrays[ATTR_TEXCOORD] = (char*)&texcoords[0];
	}
	if (!normals.empty()) {
		Attribute& attr = attributes[ATTR_NORMAL];
		attr.components = 3;
		attr.offset = offset;
		offset += sizeof(normals[0]);
		dataArrays[ATTR_NORMAL] = (char*)&normals[0];
	}
	vertexSize = offset;
	vertexData.resize(numVertices * vertexSize);
	for (uint i = 0; i < numVertices; ++i) {
		char* dst = &vertexData[i * vertexSize];
		for (uint a = 0; a < ATTR_MAX; ++a) {
			if (dataArrays[a]) {
				uint elementSize = attributes[a].components * sizeof(float);
				std::memcpy(dst + attributes[a].offset, dataArrays[a] + i * elementSize, elementSize);
			}
		}
	}
}

void Geometry::calculateBoundingSphere()
{
	float maxRadiusSq = 0;
	for (auto& pos : positions) {
		maxRadiusSq = glm::max(maxRadiusSq, glm::length2(pos));
	}
	bounds.radius = glm::sqrt(maxRadiusSq);
}

void Geometry::calculateBoundingBox()
{
	Bounds& bb = bounds;
	if (positions.empty()) {
		bb.min = vec3();
		bb.max = vec3();
		return;
	}

	bb.min.x = bb.max.x = positions[0].x;
	bb.min.y = bb.max.y = positions[0].y;
	bb.min.z = bb.max.z = positions[0].z;

	for (uint i = 1, len = positions.size(); i < len; ++i) {
		float x = positions[i].x;
		float y = positions[i].y;
		float z = positions[i].z;
		if (x < bb.min.x) bb.min.x = x;
		else if ( x > bb.max.x ) bb.max.x = x;
		if (y < bb.min.y) bb.min.y = y;
		else if ( y > bb.max.y ) bb.max.y = y;
		if (z < bb.min.z) bb.min.z = z;
		else if (z > bb.max.z) bb.max.z = z;
	}
}

void Geometry::calculateNormals()
{
	// Indexed elements
	if (!indices.empty()) {
		// Reset existing normals
		for (auto& normal : normals)
			normal = vec3();
		for (uint i = 0, len = indices.size(); i < len; i += 3) {
			vec3 normal = glm::triangleNormal(positions[indices[i]], positions[indices[i+1]], positions[indices[i+2]]);
			normals[indices[i+0]] += normal;
			normals[indices[i+1]] += normal;
			normals[indices[i+2]] += normal;
		}
		normalizeNormals();
	// Non-indexed elements
	} else {
		normals.resize(positions.size());
		for (uint i = 0, len = positions.size(); i < len; i += 3) {
			vec3 normal = glm::triangleNormal(positions[i], positions[i+1], positions[i+2]);
			normals[i+0] = normal;
			normals[i+1] = normal;
			normals[i+2] = normal;
		}
	}
}

void Geometry::normalizeNormals()
{
	for (auto& normal : normals)
		normal = glm::normalize(normal);
}

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
	std::vector<vec3> verts;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;
	while (std::getline(file, row)) {
		++lineNumber;
		std::istringstream srow(row);
		float x,y,z;
		std::string tempst;
		if (row.substr(0,2) == "v ") {  // Vertices
			srow >> tempst >> x >> y >> z;
			verts.push_back(vec3(x, y, z));
		} else if (row.substr(0,2) == "vt") {  // Texture Coordinates
			srow >> tempst >> x >> y;
			uvs.push_back(vec2(x, y));
		} else if (row.substr(0,2) == "vn") {  // Normals
			srow >> tempst >> x >> y >> z;
			normals.push_back(normalize(vec3(x, y, z)));
		} else if (row.substr(0,2) == "f ") {  // Faces
			srow >> tempst; // Eat away prefix
			// Parse face point's coordinate references
			for (int i = 1; srow >> tempst; ++i) {
				if (i > 3) {
					logError("Only triangle faces are supported in %s:%d", path.c_str(), lineNumber);
					break;
				}
				std::vector<std::string> indices = split(tempst, '/');
				if (indices.size() != 3) {
					logError("Invalid face definition in %s:%d", path.c_str(), lineNumber);
					continue;
				}
				vertices.emplace_back();
				Vertex& v = vertices.back();
				// Vertex indices are 1-based in the file
				if (!indices[0].empty()) v.position = verts.at(std::stoi(indices[0]) - 1);
				if (!indices[1].empty()) v.texcoord = uvs.at(std::stoi(indices[1]) - 1);
				if (!indices[2].empty()) v.normal = normals.at(std::stoi(indices[2]) - 1);
			}
		}
	}
}

Geometry::Geometry(const Image& heightmap)
{
	int wpoints = heightmap.width + 1;
	for (int j = 0; j <= heightmap.height; ++j) {
		for (int i = 0; i <= heightmap.width; ++i) {
			// Create the vertex
			float x = i - heightmap.width * 0.5f;
			float z = j - heightmap.height * 0.5f;
			float y = heightmap.data[heightmap.channels * (j * heightmap.width + i)] / 255.0f;
			vertices.emplace_back();
			Vertex& v = vertices.back();
			v.position = vec3(x, y, z);
			v.texcoord = vec2((float)i / heightmap.width, (float)j / heightmap.height);
			v.normal = vec3(0, 1, 0);
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
}

Geometry::~Geometry()
{
	ASSERT(!vao && !vbo && !ebo);
}

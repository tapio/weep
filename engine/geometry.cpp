#include "geometry.hpp"
#include "image.hpp"
#include "physics.hpp"
#include "iqm/iqm.h"
#include "glrenderer/glutil.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/normal.hpp>
#include <sstream>
#include <fstream>
#include <cstring>
#include "utils.hpp"

namespace {
	mat3x4 invert(mat3x4 mat) {
		mat3 invrot(vec3(mat[0].x, mat[1].x, mat[2].x), vec3(mat[0].y, mat[1].y, mat[2].y), vec3(mat[0].z, mat[1].z, mat[2].z));
		invrot[0] /= glm::length2(invrot[0]);
		invrot[1] /= glm::length2(invrot[1]);
		invrot[2] /= glm::length2(invrot[2]);
		vec3 trans(mat[0].w, mat[1].w, mat[2].w);
		mat3x4 ret;
		ret[0] = vec4(invrot[0], -glm::dot(invrot[0], trans));
		ret[1] = vec4(invrot[1], -glm::dot(invrot[1], trans));
		ret[2] = vec4(invrot[2], -glm::dot(invrot[2], trans));
		return ret;
	}

	mat3x4 jointToMatrix(quat rot, vec3 scale, vec3 transl) {
		float x = rot.x, y = rot.y, z = rot.z, w = rot.w,
			tx = 2*x, ty = 2*y, tz = 2*z,
			txx = tx*x, tyy = ty*y, tzz = tz*z,
			txy = tx*y, txz = tx*z, tyz = ty*z,
			twx = w*tx, twy = w*ty, twz = w*tz;
		vec3 a = vec3(1 - (tyy + tzz), txy - twz, txz + twy);
		vec3 b = vec3(txy + twz, 1 - (txx + tzz), tyz - twx);
		vec3 c = vec3(txz - twy, tyz + twx, 1 - (txx + tyy));
		return mat3x4(vec4(a * scale, transl.x), vec4(b * scale, transl.y), vec4(c * scale, transl.z));
	}
}

Geometry::Geometry(const string& path)
{
	START_MEASURE(geomLoadTimeMs);

	if (utils::endsWith(path, ".obj")) loadObj(path);
	else if (utils::endsWith(path, ".iqm")) loadIqm(path);
	else {
		END_CPU_SAMPLE()
		logError("Unsupported file format for geometry %s", path.c_str());
		return;
	}

	calculateBoundingSphere();
	calculateBoundingBox();
	if (!batches.empty() && batches.back().normals.empty())
		calculateNormals();
	else normalizeNormals();
	for (auto& batch : batches)
		batch.setupAttributes();
	END_MEASURE(geomLoadTimeMs)
	logDebug("Loaded mesh %s in %.1fms with %d batches, %d bones, %d anims, bound r: %f",
		path.c_str(), geomLoadTimeMs, batches.size(), bones.size(), animations.size(), bounds.radius);
}

Geometry::Geometry(const Image& heightmap)
{
	batches.emplace_back();
	Batch& batch = batches.back();
	auto& indices = batch.indices;
	auto& positions = batch.positions;
	auto& texcoords = batch.texcoords;
	auto& normals = batch.normals;

	int wpoints = heightmap.width + 1;
	int numVerts = wpoints * (heightmap.height + 1);
	positions.resize(numVerts);
	texcoords.resize(numVerts);
	normals.resize(numVerts);
	for (int j = 0; j < heightmap.height; ++j) {
		for (int i = 0; i < heightmap.width; ++i) {
			// Create the vertex
			float x = i - heightmap.width * 0.5f;
			float z = j - heightmap.height * 0.5f;
			float y = heightmap.data[heightmap.channels * (j * heightmap.width + i)] / 255.0f;
			int vert = j * wpoints + i;
			positions[vert] = vec3(x, y, z);
			texcoords[vert] = vec2((float)i / heightmap.width, (float)j / heightmap.height);
			normals[vert] = up_axis;
			// Indexed faces
			if (i == heightmap.width-1 || j == heightmap.height-1)
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
	batch.setupAttributes();
}

Geometry::Geometry(uint numParticleQuads)
{
	batches.push_back({});
	Batch& batch = batches.back();
	batch.name = "ParticleBatch";
	batch.positions.resize(numParticleQuads * 4);
	for (uint i = 0; i < batch.positions.size(); i += 4) {
		batch.positions[i+0] = {-0.5f, -0.5f, 0.f};
		batch.positions[i+1] = {-0.5f,  0.5f, 0.f};
		batch.positions[i+2] = { 0.5f,  0.5f, 0.f};
		batch.positions[i+3] = { 0.5f, -0.5f, 0.f};
	}
	batch.texcoords.resize(numParticleQuads * 4);
	for (uint i = 0; i < batch.texcoords.size(); i += 4) {
		batch.texcoords[i+0] = { 0.f, 1.f };
		batch.texcoords[i+1] = { 0.f, 0.f };
		batch.texcoords[i+2] = { 1.f, 0.f };
		batch.texcoords[i+3] = { 1.f, 1.f };
	}
	batch.indices.resize(numParticleQuads * 6);
	for (uint i = 0; i < batch.indices.size(); i += 6) {
		uint base = i / 6 * 4;
		batch.indices[i+0] = base + 0;
		batch.indices[i+1] = base + 2;
		batch.indices[i+2] = base + 1;
		batch.indices[i+3] = base + 0;
		batch.indices[i+4] = base + 3;
		batch.indices[i+5] = base + 2;
	}

	batch.setupAttributes();

	// TODO: What should this be...?
	bounds.min = { -10, -10, -10 };
	bounds.max = { 10, 10, 10 };
	bounds.radius = 10;
}

Geometry::~Geometry()
{
	if (collisionMesh)
		delete collisionMesh;
}

bool Geometry::loadObj(const string& path)
{
	uint lineNumber = 0;
	std::string row;
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		logError("Failed to open file %s", path.c_str());
		return false;
	}

	batches.emplace_back();
	Batch* batch = &batches.back();

	std::map<string, uint> mtlMap;
	std::vector<vec3> tmpVerts;
	std::vector<vec2> tmpUvs;
	std::vector<vec3> tmpNormals;
	while (std::getline(file, row)) {
		++lineNumber;
		std::istringstream srow(row);
		float x,y,z;
		std::string tempst;
		if (row.substr(0, 7) == "usemtl ") { // Material
			std::string materialName;
			srow >> tempst >> materialName;
			if (mtlMap.empty()) {
				mtlMap[materialName] = 0;
				batch->name = materialName;
			} else if (mtlMap.find(materialName) == mtlMap.end()) {
				uint newIndex = batches.size();
				mtlMap[materialName] = newIndex;
				batches.emplace_back();
				batch = &batches.back();
				batch->materialIndex = newIndex;
				batch->name = materialName;
			} else {
				batch = &batches.at(mtlMap[materialName]);
			}
		} else if (row.substr(0,2) == "v ") {  // Vertices
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
				std::vector<std::string> indices = utils::split(tempst, "/");
				if (indices.size() == 0 || indices.size() > 3) {
					logError("Invalid face definition in %s:%d", path.c_str(), lineNumber);
					continue;
				} else if (indices.size() < 3) {
					indices.resize(3);
				}
				// Vertex indices are 1-based in the file
				if (i <= 3) {
					if (!indices[0].empty()) batch->positions.push_back(tmpVerts.at(std::stoi(indices[0]) - 1));
					if (!indices[1].empty()) batch->texcoords.push_back(tmpUvs.at(std::stoi(indices[1]) - 1));
					if (!indices[2].empty()) batch->normals.push_back(tmpNormals.at(std::stoi(indices[2]) - 1));
				} else if (i == 4) {
					// Manually create a new triangle
					if (!indices[0].empty()) {
						size_t lastPos = batch->positions.size();
						batch->positions.push_back(tmpVerts.at(std::stoi(indices[0]) - 1));
						batch->positions.push_back(batch->positions.at(lastPos - 3));
						batch->positions.push_back(batch->positions.at(lastPos - 1));
					}
					if (!indices[1].empty()) {
						size_t lastPos = batch->texcoords.size();
						batch->texcoords.push_back(tmpUvs.at(std::stoi(indices[1]) - 1));
						batch->texcoords.push_back(batch->texcoords.at(lastPos - 3));
						batch->texcoords.push_back(batch->texcoords.at(lastPos - 1));
					}
					if (!indices[2].empty()) {
						size_t lastPos = batch->normals.size();
						batch->normals.push_back(tmpNormals.at(std::stoi(indices[2]) - 1));
						batch->normals.push_back(batch->normals.at(lastPos - 3));
						batch->normals.push_back(batch->normals.at(lastPos - 1));
					}
				}
			}
		}
	}
	return true;
}

template<class vector_t, class T = typename vector_t::value_type>
static bool iqmAssign(vector_t& dst, uint8* data, uint wantedFormat, uint wantedSize, iqmvertexarray& va, iqmmesh& mesh)
{
	if (va.format != wantedFormat || va.size != wantedSize) {
		logWarning("Unsupported iqm vertex array type");
		return false;
	}
	ASSERT(wantedFormat == IQM_UBYTE || wantedFormat == IQM_FLOAT);
	uint compSize = wantedFormat == IQM_UBYTE ? sizeof(uint8) : sizeof(float);
	uint start = va.offset + va.size * compSize * mesh.first_vertex;
	uint end = start + va.size * compSize * mesh.num_vertexes;
	dst.assign((T*)&data[start], (T*)&data[end]);
	ASSERT(dst.size() == mesh.num_vertexes);
	return true;
}

bool Geometry::loadIqm(const string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		logError("Failed to open file %s", path.c_str());
		return false;
	}
	iqmheader header;
	file.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (file.fail() || memcmp(header.magic, IQM_MAGIC, sizeof(header.magic))) {
		logError("File %s is not in IQM format", path.c_str());
		return false;
	}
	if (header.version != IQM_VERSION) {
		logError("Unsupported IQM version %u (expected %d) in %s", header.version, IQM_VERSION, path.c_str());
		return false;
	}
	std::vector<uint8> data;
	data.resize(header.filesize);
	file.read(reinterpret_cast<char*>(&data[sizeof(header)]), header.filesize - sizeof(header));
	if (file.fail()) {
		logError("Failed to read data from %s", path.c_str());
		return false;
	}

	iqmvertexarray* vas = (iqmvertexarray*)&data[header.ofs_vertexarrays];
	iqmmesh* meshes = (iqmmesh*)&data[header.ofs_meshes];
	iqmtriangle* tris = (iqmtriangle*)&data[header.ofs_triangles];
	iqmjoint* joints = (iqmjoint*)&data[header.ofs_joints];
	const char *str = header.ofs_text ? (char*)&data[header.ofs_text] : "";
	ASSERT(header.num_triangles);
	std::map<string, uint> mtlMap;

	for (uint m = 0; m < header.num_meshes; ++m) {
		batches.emplace_back();
		Batch& batch = batches.back();
		iqmmesh& mesh = meshes[m];
		batch.name = &str[mesh.name];
		batch.indices.assign((uint*)&tris[mesh.first_triangle], (uint*)&tris[mesh.first_triangle + mesh.num_triangles]);
		string materialName = &str[mesh.material];
		if (mtlMap.empty()) {
			mtlMap[materialName] = 0;
		} else if (mtlMap.find(materialName) == mtlMap.end()) {
			uint newIndex = mtlMap.size();
			mtlMap[materialName] = newIndex;
			batch.materialIndex = newIndex;
		} else {
			batch.materialIndex = mtlMap[materialName];
		}
		// Fix triangle winding
		for (uint i = 0; i < mesh.num_triangles; ++i)
			std::swap(batch.indices[i*3], batch.indices[i*3+2]);
		// Fix index offsets
		for (auto& index : batch.indices)
			index -= mesh.first_vertex;
		// Assign vertices
		for (uint i = 0; i < header.num_vertexarrays; ++i) {
			iqmvertexarray &va = vas[i];
			switch (va.type) {
				case IQM_POSITION:
					iqmAssign(batch.positions, &data[0], IQM_FLOAT, 3, va, mesh);
					break;
				case IQM_TEXCOORD:
					iqmAssign(batch.texcoords, &data[0], IQM_FLOAT, 2, va, mesh);
					break;
				case IQM_NORMAL:
					iqmAssign(batch.normals, &data[0], IQM_FLOAT, 3, va, mesh);
					break;
				case IQM_TANGENT:
					iqmAssign(batch.tangents, &data[0], IQM_FLOAT, 3, va, mesh);
					break;
				case IQM_BLENDINDEXES:
					iqmAssign(batch.boneindices, &data[0], IQM_UBYTE, 4, va, mesh);
					break;
				case IQM_BLENDWEIGHTS:
					iqmAssign(batch.boneweights, &data[0], IQM_UBYTE, 4, va, mesh);
					break;
				case IQM_COLOR:
					iqmAssign(batch.colors, &data[0], IQM_UBYTE, 4, va, mesh);
					break;
			}
		}
	}

	std::vector<mat3x4> inverseBones;
	bones.resize(header.num_joints);
	boneParents.resize(header.num_joints);
	inverseBones.resize(header.num_joints);
	for (uint i = 0; i < header.num_joints; ++i) {
		iqmjoint &j = joints[i];
		boneParents[i] = j.parent;
		quat rot = normalize(quat(j.rotate[3], j.rotate[0], j.rotate[1], j.rotate[2]));
		vec3 scale(j.scale[0], j.scale[1], j.scale[2]);
		vec3 transl(j.translate[0], j.translate[1], j.translate[2]);
		bones[i] = jointToMatrix(rot, scale, transl);
		inverseBones[i] = invert(bones[i]);
		if (j.parent >= 0) {
			bones[i] = multiplyBones(bones[j.parent], bones[i]);
			inverseBones[i] = multiplyBones(inverseBones[i], inverseBones[j.parent]);
		}
	}

	iqmanim* anims = (iqmanim*)&data[header.ofs_anims];
	iqmpose* poses = (iqmpose*)&data[header.ofs_poses];
	ushort* framedata = (ushort*)&data[header.ofs_frames];
	animFrames.resize(header.num_frames * header.num_poses);

	for (uint i = 0; i < header.num_frames; ++i) {
		for (uint j = 0; j < header.num_poses; ++j) {
			iqmpose &p = poses[j];
			quat rotate;
			vec3 translate, scale;
			translate.x = p.channeloffset[0]; if (p.mask&0x01) translate.x += *framedata++ * p.channelscale[0];
			translate.y = p.channeloffset[1]; if (p.mask&0x02) translate.y += *framedata++ * p.channelscale[1];
			translate.z = p.channeloffset[2]; if (p.mask&0x04) translate.z += *framedata++ * p.channelscale[2];
			rotate.x = p.channeloffset[3]; if (p.mask&0x08) rotate.x += *framedata++ * p.channelscale[3];
			rotate.y = p.channeloffset[4]; if (p.mask&0x10) rotate.y += *framedata++ * p.channelscale[4];
			rotate.z = p.channeloffset[5]; if (p.mask&0x20) rotate.z += *framedata++ * p.channelscale[5];
			rotate.w = p.channeloffset[6]; if (p.mask&0x40) rotate.w += *framedata++ * p.channelscale[6];
			scale.x = p.channeloffset[7]; if (p.mask&0x80) scale.x += *framedata++ * p.channelscale[7];
			scale.y = p.channeloffset[8]; if (p.mask&0x100) scale.y += *framedata++ * p.channelscale[8];
			scale.z = p.channeloffset[9]; if (p.mask&0x200) scale.z += *framedata++ * p.channelscale[9];
			mat3x4 matrix = jointToMatrix(rotate, scale, translate);
			if (p.parent >= 0) animFrames[i * header.num_poses + j] = multiplyBones(multiplyBones(bones[p.parent], matrix), inverseBones[j]);
			else animFrames[i * header.num_poses + j] = multiplyBones(matrix, inverseBones[j]);
		}
	}

	for (uint i = 0; i < header.num_anims; ++i) {
		iqmanim &a = anims[i];
		Animation anim;
		anim.frameRate = a.framerate;
		anim.start = a.first_frame;
		anim.length = a.num_frames;
		anim.name = &str[a.name];
		animations.push_back(std::move(anim));
	}

	return true;
}

void Geometry::calculateBoundingSphere()
{
	float maxRadiusSq = 0;
	for (auto& batch : batches)
		for (auto& pos : batch.positions)
			maxRadiusSq = glm::max(maxRadiusSq, glm::length2(pos));
	bounds.radius = glm::sqrt(maxRadiusSq);
}

void Geometry::calculateBoundingBox()
{
	Bounds& bb = bounds;
	bb.min = vec3();
	bb.max = vec3();

	for (auto& batch : batches) {
		auto& positions = batch.positions;
		for (uint i = 0, len = positions.size(); i < len; ++i) {
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
}

void Geometry::calculateNormals()
{
	for (auto& batch : batches) {
		auto& indices = batch.indices;
		auto& positions = batch.positions;
		auto& normals = batch.normals;
		// Indexed elements
		if (!indices.empty()) {
			// Reset existing normals
			normals.clear();
			normals.resize(positions.size());
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
}

void Geometry::calculateTangents()
{
	for (auto& batch : batches) {
		auto& indices = batch.indices;
		auto& positions = batch.positions;
		auto& texcoords = batch.texcoords;
		auto& normals = batch.normals;
		auto& tangents = batch.tangents;
		// Indexed elements
		if (!indices.empty()) {
			// Reset existing tangents
			//tangents.clear();
			//tangents.resize(positions.size());
			ASSERT(!"Indexed tangents not implemented");
		// Non-indexed elements
		} else {
			std::vector<vec3> bitangents;
			bitangents.resize(positions.size());
			tangents.resize(positions.size());
			for (uint i = 0, len = positions.size(); i < len; i += 3) {
				// Edges of the triangle : postion delta
				vec3 deltaPos1 = positions[i+1] - positions[i];
				vec3 deltaPos2 = positions[i+2] - positions[i];

				// UV delta
				vec2 deltaUV1 = texcoords[i+1] - texcoords[i];
				vec2 deltaUV2 = texcoords[i+2] - texcoords[i];

				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

				for (int j = 0; j < 3; ++j) {
					vec3 n = normals[i+j];
					// Gram-Schmidt orthogonalize
					vec3 t = glm::normalize(tangent - n * glm::dot(n, tangent));
					// Calculate handedness
					if (glm::dot(glm::cross(n, t), bitangent) < 0.0f)
						t = t * -1.0f;
					tangents[i+j] = t;
				}
			}
		}
	}

}

void Geometry::normalizeNormals()
{
	for (auto& batch : batches)
		for (auto& normal : batch.normals)
			normal = glm::normalize(normal);
}

void Geometry::applyMatrix(mat4 transform)
{
	mat3 normalTransform = mat3(glm::inverseTranspose(transform));
	for (auto& batch : batches) {
		for (auto& pos : batch.positions)
			pos = vec3(transform * vec4(pos, 1.0f));
		for (auto& normal : batch.normals)
			normal = normalTransform * normal;
	}
}

void Geometry::generateCollisionTriMesh(bool deduplicateVertices)
{
	ASSERT(!collisionMesh);
	uint numVerts = 0;
	for (auto& batch : batches) {
		if (!batch.indices.empty()) numVerts += batch.indices.size();
		else if (!batch.positions.empty()) numVerts += batch.positions.size();
		else if (!batch.positions2d.empty()) numVerts += batch.positions2d.size();
	}
	collisionMesh = new btTriangleMesh(numVerts > 65536);

	for (auto& batch : batches) {
		if (batch.indices.empty()) {
			for (uint i = 0; i < batch.positions.size(); i += 3) {
				collisionMesh->addTriangle(
					convert(batch.positions[i]),
					convert(batch.positions[i+1]),
					convert(batch.positions[i+2]),
					deduplicateVertices);
			}
		} else {
			for (uint i = 0; i < batch.indices.size(); i += 3) {
				collisionMesh->addTriangle(
					convert(batch.positions[batch.indices[i]]),
					convert(batch.positions[batch.indices[i+1]]),
					convert(batch.positions[batch.indices[i+2]]),
					deduplicateVertices);
			}
		}
	}
}

void Geometry::merge(const Geometry& geometry, vec3 offset, int materialIndexOffset)
{
	for (auto& b : geometry.batches) {
		batches.push_back(b);
		Batch& batch = batches.back();
		batch.materialIndex += materialIndexOffset;
		for (auto& pos : batch.positions)
			pos += offset;
	}
}


// Batch

Batch::~Batch()
{
	ASSERT(renderId == -1);
}

void Batch::setupAttributes()
{
	ASSERT(positions.empty() || positions2d.empty());
	int offset = 0;
	const char* dataArrays[ATTR_MAX] = {0};
	if (!positions.empty()) {
		Attribute& attr = attributes[ATTR_POSITION];
		attr.components = 3;
		attr.type = GL_FLOAT;
		attr.offset = offset;
		offset += sizeof(positions[0]);
		numVertices = positions.size();
		dataArrays[ATTR_POSITION] = (char*)&positions[0];
	}
	if (!positions2d.empty()) {
		Attribute& attr = attributes[ATTR_POSITION];
		attr.components = 2;
		attr.type = GL_FLOAT;
		attr.offset = offset;
		offset += sizeof(positions2d[0]);
		numVertices = positions2d.size();
		dataArrays[ATTR_POSITION] = (char*)&positions2d[0];
	}
	if (!texcoords.empty()) {
		Attribute& attr = attributes[ATTR_TEXCOORD];
		attr.components = 2;
		attr.type = GL_FLOAT;
		attr.offset = offset;
		offset += sizeof(texcoords[0]);
		dataArrays[ATTR_TEXCOORD] = (char*)&texcoords[0];
	}
	if (!normals.empty()) {
		Attribute& attr = attributes[ATTR_NORMAL];
		attr.components = 3;
		attr.type = GL_FLOAT;
		attr.offset = offset;
		offset += sizeof(normals[0]);
		dataArrays[ATTR_NORMAL] = (char*)&normals[0];
	}
	if (!tangents.empty()) {
		Attribute& attr = attributes[ATTR_TANGENT];
		attr.components = 3;
		attr.type = GL_FLOAT;
		attr.offset = offset;
		offset += sizeof(tangents[0]);
		dataArrays[ATTR_TANGENT] = (char*)&tangents[0];
	}
	if (!boneindices.empty()) {
		Attribute& attr = attributes[ATTR_BONE_INDEX];
		attr.components = 4;
		attr.type = GL_UNSIGNED_BYTE;
		attr.offset = offset;
		offset += sizeof(boneindices[0]);
		dataArrays[ATTR_BONE_INDEX] = (char*)&boneindices[0];
	}
	if (!boneweights.empty()) {
		Attribute& attr = attributes[ATTR_BONE_WEIGHT];
		attr.components = 4;
		attr.type = GL_UNSIGNED_BYTE;
		attr.offset = offset;
		attr.normalized = true;
		offset += sizeof(boneweights[0]);
		dataArrays[ATTR_BONE_WEIGHT] = (char*)&boneweights[0];
	}
	vertexSize = offset;
	vertexData.resize(numVertices * vertexSize);
	for (uint i = 0; i < numVertices; ++i) {
		char* dst = &vertexData[i * vertexSize];
		for (uint a = 0; a < ATTR_MAX; ++a) {
			if (dataArrays[a]) {
				uint elementSize = attributes[a].components * glutil::getTypeSize(attributes[a].type);
				std::memcpy(dst + attributes[a].offset, dataArrays[a] + i * elementSize, elementSize);
			}
		}
	}
}

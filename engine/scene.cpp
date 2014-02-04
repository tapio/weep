#include "scene.hpp"
#include "material.hpp"
#include "geometry.hpp"
#include "resources.hpp"

namespace {
	vec3 toVec3(const Json& arr) {
		ASSERT(arr.is_array());
		return vec3(arr[0].number_value(), arr[1].number_value(), arr[2].number_value());
	}
}

void Scene::load(const string& path, Resources& resources)
{
	std::string err;
	Json jsonScene = Json::parse(readFile(path), err);
	if (!err.empty())
		panic("Failed to read scene: %s", err.c_str());
	ASSERT(jsonScene.is_array());
	for (uint i = 0; i < jsonScene.array_items().size(); ++i) {
		const Json& def = jsonScene[i];
		ASSERT(def.is_object());
		m_models.emplace_back(Model());
		Model& model = m_models.back();
		if (!def["material"].is_null()) {
			const Json& materialDef = def["material"];
			ASSERT(materialDef.is_object());
			model.material.reset(new Material());
			if (!materialDef["ambient"].is_null())
				model.material->ambient = toVec3(materialDef["ambient"]);
			if (!materialDef["diffuse"].is_null())
				model.material->diffuse = toVec3(materialDef["diffuse"]);
			if (!materialDef["specular"].is_null())
				model.material->specular = toVec3(materialDef["specular"]);
			if (!materialDef["diffuseMap"].is_null())
				model.material->diffuseMap = resources.getImage(materialDef["diffuseMap"].string_value());
			model.material->shaderName = materialDef["shaderName"].string_value();
		}
		if (!def["geometry"].is_null()) {
			const Json& defGeom = def["geometry"];
			if (defGeom.is_string())
				model.geometry = resources.getGeometry(defGeom.string_value());
			else model.geometry = resources.getHeightmap(defGeom["heightmap"].string_value());
		}
		if (!def["position"].is_null()) {
			model.position = toVec3(def["position"]);
		}
		if (!def["rotation"].is_null()) {
			model.rotation = quat(toVec3(def["rotation"]));
		}
		if (!def["scale"].is_null()) {
			const Json& scaleDef = def["scale"];
			model.scale = scaleDef.is_number() ? vec3(scaleDef.number_value()) : toVec3(scaleDef);
		}
	}
	logDebug("Loaded scene with %d models", m_models.size());
}

void Scene::reset()
{
	m_models.clear();
}

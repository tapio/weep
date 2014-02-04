#include "scene.hpp"
#include "material.hpp"
#include "geometry.hpp"
#include "resources.hpp"

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
			//model.material->ambient = vec3(0.2f, 0.2f, 0.2f);
			//model.material->diffuse = vec3(0.0f, 0.0f, 0.3f);
			if (!materialDef["diffuseMap"].is_null())
				model.material->diffuseMap = resources.getImage(materialDef["diffuseMap"].string_value());
			model.material->shaderName = materialDef["shaderName"].string_value();
		}
		if (!def["geometry"].is_null()) {
			const string& geomPath = def["geometry"].string_value();
			model.geometry = resources.getGeometry(geomPath);
		}
		if (!def["position"].is_null()) {
			const Json& posDef = def["position"];
			ASSERT(posDef.is_array());
			vec3 pos(posDef[0].number_value(), posDef[1].number_value(), posDef[2].number_value());
			model.transform = translate(model.transform, pos);
		}
	}
	logDebug("Loaded scene with %d models", m_models.size());
}

void Scene::reset()
{
	m_models.clear();
}

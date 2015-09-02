#include "scene.hpp"
#include "material.hpp"
#include "geometry.hpp"
#include "resources.hpp"
#include "image.hpp"
#include "engine.hpp"

namespace {

	void parseModel(Model& model, const Json& def, Resources& resources) {
		// Parse geometry
		if (!def["geometry"].is_null()) {
			const Json& defGeom = def["geometry"];
			if (defGeom.is_string())
				model.geometry = resources.getGeometry(defGeom.string_value());
			else model.geometry = resources.getHeightmap(defGeom["heightmap"].string_value());
		}

		// Parse material
		if (!def["material"].is_null()) {
			const Json& materialDef = def["material"];
			ASSERT(materialDef.is_object());
			if (!model.material)
				model.material.reset(new Material());
			else model.material.reset(new Material(*model.material));
			if (materialDef["shaderName"].is_string())
				model.material->shaderName = materialDef["shaderName"].string_value();
			if (materialDef["tessellate"].bool_value())
				model.material->tessellate = true;
			if (!materialDef["ambient"].is_null())
				model.material->ambient = colorToVec3(materialDef["ambient"]);
			if (!materialDef["diffuse"].is_null())
				model.material->diffuse = colorToVec3(materialDef["diffuse"]);
			if (!materialDef["specular"].is_null())
				model.material->specular = colorToVec3(materialDef["specular"]);
			if (!materialDef["shininess"].is_null())
				model.material->shininess = materialDef["shininess"].number_value();

			if (!materialDef["diffuseMap"].is_null()) {
				model.material->map[Material::DIFFUSE_MAP] = resources.getImage(materialDef["diffuseMap"].string_value());
				model.material->map[Material::DIFFUSE_MAP]->sRGB = true;
			}
			if (!materialDef["specularMap"].is_null()) {
				model.material->map[Material::SPECULAR_MAP] = resources.getImage(materialDef["specularMap"].string_value());
				model.material->map[Material::SPECULAR_MAP]->sRGB = true;
			}
			if (!materialDef["emissionMap"].is_null()) {
				model.material->map[Material::EMISSION_MAP] = resources.getImage(materialDef["emissionMap"].string_value());
				model.material->map[Material::EMISSION_MAP]->sRGB = true;
			}
			if (!materialDef["normalMap"].is_null())
				model.material->map[Material::NORMAL_MAP] = resources.getImage(materialDef["normalMap"].string_value());
			if (!materialDef["heightMap"].is_null())
				model.material->map[Material::HEIGHT_MAP] = resources.getImage(materialDef["heightMap"].string_value());
		}

		// Parse body
		if (!def["body"].is_null()) {
			const Json& bodyDef = def["body"];
			ASSERT(bodyDef.is_object());

			const string& shape = bodyDef["shape"].string_value();
			if (shape == "box")
				model.bodyDef.shape = Model::BodyDef::SHAPE_BOX;
			if (shape == "sphere")
				model.bodyDef.shape = Model::BodyDef::SHAPE_SPHERE;

			if (bodyDef["mass"].is_number())
				model.bodyDef.mass = bodyDef["mass"].number_value();
		}

		// Parse transform
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

		if (def["name"].is_string())
			model.name = def["name"].string_value();

		if (model.geometry) {
			model.bounds.min = model.geometry->bounds.min * model.scale;
			model.bounds.max = model.geometry->bounds.max * model.scale;
			model.bounds.radius = model.geometry->bounds.radius * glm::compMax(model.scale);
		}
	}
}

void Scene::load(const string& path, Resources& resources)
{
	uint t0 = Engine::timems();
	std::string err;
	Json jsonScene = Json::parse(resources.getText(path, Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read scene %s: %s", path.c_str(), err.c_str());

	// Parse prefabs
	if (jsonScene.is_object() && jsonScene["prefabs"].is_object()) {
		const Json::object& prefabs = jsonScene["prefabs"].object_items();
		for (auto& it : prefabs)
		{
			parseModel(m_prefabs[it.first], it.second, resources);
		}
	}

	// Parse objects
	const Json::array& objects = jsonScene.is_array() ? jsonScene.array_items() : jsonScene["objects"].array_items();
	for (uint i = 0; i < objects.size(); ++i) {
		const Json& def = objects[i];
		ASSERT(def.is_object());

		// Parse light
		const Json& lightDef = def["light"];
		if (!lightDef.is_null()) {
			m_lights.emplace_back();
			Light& light = m_lights.back();
			const string& lightType = lightDef["type"].string_value();
			if (lightType == "ambient") light.type = Light::AMBIENT_LIGHT;
			else if (lightType == "point") light.type = Light::POINT_LIGHT;
			else if (lightType == "directional") light.type = Light::DIRECTIONAL_LIGHT;
			else if (lightType == "spot") light.type = Light::SPOT_LIGHT;
			else if (lightType == "area") light.type = Light::AREA_LIGHT;
			else if (lightType == "hemisphere") light.type = Light::HEMISPHERE_LIGHT;
			else logError("Unknown light type \"%s\"", lightType.c_str());
			if (!lightDef["color"].is_null())
				light.color = colorToVec3(lightDef["color"]);
			if (!def["position"].is_null())
				light.position = toVec3(def["position"]);
			if (!lightDef["direction"].is_null())
				light.direction = toVec3(lightDef["direction"]);
			if (!lightDef["distance"].is_null())
				light.distance = lightDef["distance"].number_value();
			if (!lightDef["decay"].is_null())
				light.decay = lightDef["decay"].number_value();
		}

		Model* model = nullptr;

		if (def["prefab"].is_string()) {
			auto prefabIter = m_prefabs.find(def["prefab"].string_value());
			if (prefabIter != m_prefabs.end()) {
				m_models.push_back(prefabIter->second);
				model = &m_models.back();
			} else {
				logWarning("Could not find prefab \"%s\"", def["prefab"].string_value().c_str());
			}
		} else if (!def["geometry"].is_null()) {
			m_models.emplace_back();
			model = &m_models.back();
		}

		if (model)
			parseModel(*model, def, resources);
	}
	uint t1 = Engine::timems();
	logDebug("Loaded scene in %dms with %d models, %d lights", t1 - t0, m_models.size(), m_lights.size());
}

void Scene::reset()
{
	m_models.clear();
	m_lights.clear();
	m_prefabs.clear();
}

Model* Scene::find(const std::string& name)
{
	if (name.empty())
		return nullptr;
	for (uint i = 0, l = m_models.size(); i < l; ++i)
		if (m_models[i].name == name)
			return &m_models[i];
	return nullptr;
}

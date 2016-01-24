#include "scene.hpp"
#include "material.hpp"
#include "geometry.hpp"
#include "resources.hpp"
#include "image.hpp"
#include "engine.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include "animation.hpp"
#include "camera.hpp"
#include "components.hpp"
#include "environment.hpp"
#include "gui.hpp"
#include "module.hpp"
#include "renderer.hpp"
#include "glrenderer/renderdevice.hpp"
#include <id/id.hpp>
#include <glm/gtx/component_wise.hpp>

#define USE_DEBUG_NAMES

namespace {

	Json assign(Json lhs, const Json& rhs) {
		ASSERT(rhs.is_object());
		Json::object object;
		if (lhs.is_object())
			object = lhs.object_items();
		for (auto& item : rhs.object_items()) {
			if (item.second.is_object())
				object[item.first] = assign(lhs[item.first], item.second);
			else object[item.first] = item.second;
		}
		return Json(object);
	}

	inline vec2 toVec2(const Json& v) {
		if (v.is_number()) return vec2(v.number_value());
		ASSERT(v.is_array());
		return vec2(v[0].number_value(), v[1].number_value());

	}

	inline vec3 toVec3(const Json& v) {
		if (v.is_number()) return vec3(v.number_value());
		ASSERT(v.is_array());
		return vec3(v[0].number_value(), v[1].number_value(), v[2].number_value());
	}

	inline vec3 colorToVec3(const Json& color) {
		if (color.is_number()) return vec3(color.number_value());
		else if (color.is_string()) {
			const std::string& str = color.string_value();
			if (str.length() == 7 && str[0] == '#') {
				float r = std::stoi(str.substr(1, 2), 0, 16) / 255.f;
				float g = std::stoi(str.substr(3, 2), 0, 16) / 255.f;
				float b = std::stoi(str.substr(5, 2), 0, 16) / 255.f;
				return vec3(r, g, b);
			} else if (str.length() == 4 && str[0] == '#') {
				float r = std::stoi(str.substr(1, 1) + str.substr(1, 1), 0, 16) / 255.f;
				float g = std::stoi(str.substr(2, 1) + str.substr(2, 1), 0, 16) / 255.f;
				float b = std::stoi(str.substr(3, 1) + str.substr(3, 1), 0, 16) / 255.f;
				return vec3(r, g, b);
			} else {
				ASSERT(!"Malformed color string");
				return vec3(1, 0, 1);
			}
		}
		return toVec3(color);
	}

	template<typename T> void setNumber(T& dst, const Json& src) {
		if (src.is_number())
			dst = (T)src.number_value();
	}

	template<typename T> void setFlag(T& dst, uint flag, const Json& src) {
		if (src.is_bool()) {
			if (src.bool_value()) dst |= flag;
			else dst &= ~flag;
		}
	}

	void setVec3(vec3& dst, const Json& src) {
		if (!src.is_null())
			dst = toVec3(src);
	}

	void setColor(vec3& dst, const Json& src) {
		if (!src.is_null())
			dst = colorToVec3(src);
	}

	void setString(string& dst, const Json& src) {
		if (src.is_string())
			dst = src.string_value();
	}

	Material& parseMaterial(Material& material, const Json& def, Resources& resources) {
		ASSERT(def.is_object());
		setString(material.shaderName, def["shaderName"]);
		setFlag(material.flags, Material::TESSELLATE, def["tessellate"]);
		setFlag(material.flags, Material::CAST_SHADOW, def["castShadow"]);
		setFlag(material.flags, Material::RECEIVE_SHADOW, def["receiveShadow"]);
		setFlag(material.flags, Material::ANIMATED, def["animated"]);
		setFlag(material.flags, Material::ALPHA_TEST, def["alphaTest"]);
		setColor(material.ambient, def["ambient"]);
		setColor(material.diffuse, def["diffuse"]);
		setColor(material.specular, def["specular"]);
		setColor(material.emissive, def["emissive"]);
		setNumber(material.shininess, def["shininess"]);
		setNumber(material.reflectivity, def["reflectivity"]);
		setNumber(material.parallax, def["parallax"]);

		if (!def["uvOffset"].is_null())
			material.uvOffset = toVec2(def["uvOffset"]);
		if (!def["uvRepeat"].is_null())
			material.uvRepeat = toVec2(def["uvRepeat"]);

		if (!def["diffuseMap"].is_null()) {
			material.map[Material::DIFFUSE_MAP] = resources.getImageAsync(def["diffuseMap"].string_value());
			material.map[Material::DIFFUSE_MAP]->sRGB = true;
		}
		if (!def["specularMap"].is_null()) {
			material.map[Material::SPECULAR_MAP] = resources.getImageAsync(def["specularMap"].string_value());
			material.map[Material::SPECULAR_MAP]->sRGB = true;
		}
		if (!def["emissionMap"].is_null()) {
			material.map[Material::EMISSION_MAP] = resources.getImageAsync(def["emissionMap"].string_value());
			material.map[Material::EMISSION_MAP]->sRGB = true;
		}
		if (!def["normalMap"].is_null())
			material.map[Material::NORMAL_MAP] = resources.getImageAsync(def["normalMap"].string_value());
		if (!def["heightMap"].is_null())
			material.map[Material::HEIGHT_MAP] = resources.getImageAsync(def["heightMap"].string_value());
		if (!def["aoMap"].is_null())
			material.map[Material::AO_MAP] = resources.getImageAsync(def["aoMap"].string_value());
		if (!def["reflectionMap"].is_null())
			material.map[Material::REFLECTION_MAP] = resources.getImageAsync(def["reflectionMap"].string_value());

		return material;
	}

	void parseModel(Model& model, const Json& def, Resources& resources) {
		// Parse geometry
		if (!def["geometry"].is_null()) {
			const Json& defGeom = def["geometry"];
			if (defGeom.is_string())
				model.geometry = resources.getGeometry(defGeom.string_value());
			else model.geometry = resources.getHeightmap(defGeom["heightmap"].string_value());
		}

		// Parse material
		const Json& materialDef = def["material"];
		if (materialDef.is_object()) {
			ASSERT(model.materials.size() <= 1);
			if (model.materials.empty())
				model.materials.emplace_back();
			parseMaterial(model.materials.back(), materialDef, resources);
		} else if (materialDef.is_array()) {
			if (model.materials.empty()) {
				for (auto& matDef : materialDef.array_items()) {
					model.materials.emplace_back();
					parseMaterial(model.materials.back(), matDef, resources);
				}
			} else {
				ASSERT(model.materials.size() == materialDef.array_items().size());
				for (uint i = 0; i < materialDef.array_items().size(); ++i)
					parseMaterial(model.materials[i], materialDef[i], resources);
			}
		}
	}

	Environment parseEnvironment(const Json& def, Resources& resources)
	{
		Environment env;
		ASSERT(def.is_object());
		if (def["skybox"].is_string()) {
			env.skyType = Environment::SKY_SKYBOX;
			const string& skyboxPath = def["skybox"].string_value();
			if (skyboxPath.back() == '/' || skyboxPath.back() == '\\') {
				env.skybox[0] = resources.getImage(skyboxPath + "px.jpg");
				env.skybox[1] = resources.getImage(skyboxPath + "nx.jpg");
				env.skybox[2] = resources.getImage(skyboxPath + "py.jpg");
				env.skybox[3] = resources.getImage(skyboxPath + "ny.jpg");
				env.skybox[4] = resources.getImage(skyboxPath + "pz.jpg");
				env.skybox[5] = resources.getImage(skyboxPath + "nz.jpg");
			} else {
				for (int i = 0; i < 6; i++)
					env.skybox[i] = resources.getImage(skyboxPath);
			}
		} else if (def["skybox"].is_array()) {
			env.skyType = Environment::SKY_SKYBOX;
			for (int i = 0; i < 6; i++)
				env.skybox[i] = resources.getImage(def["skybox"][i].string_value());
		}
		for (int i = 0; i < 6; i++)
			if (env.skybox[i])
				env.skybox[i]->sRGB = true;

		setNumber(env.exposure, def["exposure"]);
		setNumber(env.shadowDarkness, def["shadowDarkness"]);
		setNumber(env.bloomThreshold, def["bloomThreshold"]);
		setNumber(env.bloomIntensity, def["bloomIntensity"]);
		setNumber(env.tonemap, def["tonemap"]);
		setColor(env.ambient, def["ambient"]);
		if (!def["sunPosition"].is_null())
			env.sunPosition = toVec3(def["sunPosition"]);
		setColor(env.sunColor, def["sunColor"]);
		setColor(env.fogColor, def["fogColor"]);
		setNumber(env.fogDensity, def["fogDensity"]);
		return env;
	}
}

void SceneLoader::load(const string& path, Resources& resources)
{
	uint t0 = Engine::timems();
	m_environment = Json();
	load_internal(path, resources);

	if (!m_environment.is_null() && world->has_system<RenderSystem>()) {
		RenderSystem& renderer = world->get_system<RenderSystem>();
		renderer.env() = parseEnvironment(m_environment, resources);
		renderer.device().setEnvironment(&renderer.env());
	}

	Entity cameraEnt;
	if (world->has_tagged_entity("camera")) {
		cameraEnt = world->get_entity_by_tag("camera");
	} else {
		cameraEnt = world->create();
		cameraEnt.tag("camera");
	}
	if (!cameraEnt.has<Camera>()) {
		cameraEnt.add<Camera>();
		Camera& camera = cameraEnt.get<Camera>();
		float ar = Engine::width() / (float)Engine::height();
		camera.makePerspective(45, ar, 0.1, 1000);
		if (cameraEnt.has<Transform>()) {
			Transform& trans = cameraEnt.get<Transform>();
			// TODO: Should probably not have pos/rot duplicated
			camera.position = trans.position;
			camera.rotation = trans.rotation;
			camera.updateViewMatrix();
		} else {
			camera.view = glm::lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0));
		}
	}

	resources.startAsyncLoading();

	uint t1 = Engine::timems();
	logDebug("Loaded scene in %dms with %d models, %d bodies, %d lights, %d prefabs", t1 - t0, numModels, numBodies, numLights, prefabs.size());
}

void SceneLoader::load_internal(const string& path, Resources& resources)
{
	std::string err;
	Json jsonScene = Json::parse(resources.getText(path, Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read scene %s: %s", path.c_str(), err.c_str());

	if (jsonScene.is_object()) {
		// Handle includes
		if (jsonScene["include"].is_string()) {
			load_internal(jsonScene["include"].string_value(), resources);
		} else if (jsonScene["include"].is_array()) {
			for (auto& includePath : jsonScene["include"].array_items())
				load_internal(includePath.string_value(), resources);
		}

		// Parse modules
		if (jsonScene["modules"].is_array() && world->has_system<ModuleSystem>()) {
			world->get_system<ModuleSystem>().load(jsonScene["modules"], false);
		}

		// Parse environment
		if (jsonScene["environment"].is_object()) {
			m_environment = assign(m_environment, jsonScene["environment"]);
		}

		// Parse fonts
		if (jsonScene["fonts"].is_object() && world->has_system<ImGuiSystem>()) {
			const Json::object& fonts = jsonScene["fonts"].object_items();
			ImGuiSystem& imgui = world->get_system<ImGuiSystem>();
			for (auto& it : fonts) {
				ASSERT(it.second.is_object());
				string path = resources.findPath(it.second["path"].string_value());
				float size = it.second["size"].number_value();
				imgui.loadFont(it.first, path, size);
			}
		}

		// Parse sounds
		if (jsonScene["sounds"].is_object() && world->has_system<AudioSystem>()) {
			const Json::object& sounds = jsonScene["sounds"].object_items();
			AudioSystem& audio = world->get_system<AudioSystem>();
			for (auto& it : sounds) {
				if (it.second.is_string()) {
					audio.add(it.first, resources.getBinary(it.second.string_value()));
				} else if (it.second.is_array()) {
					for (auto& it2 : it.second.array_items()) {
						ASSERT(it2.is_string());
						audio.add(it.first, resources.getBinary(it2.string_value()));
					}
				}
			}
		}

		// Parse prefabs
		if (jsonScene["prefabs"].is_object()) {
			const Json::object& scenePrefabs = jsonScene["prefabs"].object_items();
			for (auto& it : scenePrefabs)
				prefabs[it.first] = it.second;
		}
	}

	// Parse objects
	const Json::array& objects = jsonScene.is_array() ? jsonScene.array_items() : jsonScene["objects"].array_items();
	for (uint i = 0; i < objects.size(); ++i) {
		instantiate(objects[i], resources);
	}
}

Entity SceneLoader::instantiate(Json def, Resources& resources)
{
	ASSERT(def.is_object());

	if (def["prefab"].is_string()) {
		const string& prefabName = def["prefab"].string_value();
		auto prefabIter = prefabs.find(prefabName);
		if (prefabIter != prefabs.end()) {
			def = assign(prefabIter->second, def);
		} else if (endsWith(prefabName, ".json")) {
				string err;
				Json extPrefab = Json::parse(resources.getText(prefabName, Resources::NO_CACHE), err);
				if (!err.empty()) {
					logError("Failed to parse prefab \"%s\": %s", prefabName.c_str(), err.c_str());
				} else {
					def = assign(extPrefab, def);
					prefabs[prefabName] = extPrefab;
				}
		} else {
			logError("Could not find prefab \"%s\"", prefabName.c_str());
		}
	}

	Entity entity = world->create();

	if (def["name"].is_string()) {
		entity.tag(def["name"].string_value());
	} else {
#ifdef USE_DEBUG_NAMES
		static uint debugId = 0;
		string name;
		if (def["prefab"].is_string())
			name = def["prefab"].string_value() + "#";
		else name = "object#";
		name += std::to_string(debugId++);
		entity.tag(name);
#endif
	}

	// Parse transform
	if (!def["position"].is_null() || !def["rotation"].is_null() || !def["scale"].is_null() || !def["geometry"].is_null()) {
		Transform transform;
		setVec3(transform.position, def["position"]);
		setVec3(transform.scale, def["scale"]);
		if (!def["rotation"].is_null())
			transform.rotation = quat(toVec3(def["rotation"]));
		entity.add(transform);
	}

	// Parse light
	const Json& lightDef = def["light"];
	if (!lightDef.is_null()) {
		Light light;
		const string& lightType = lightDef["type"].string_value();
		if (lightType == "ambient") light.type = Light::AMBIENT_LIGHT;
		else if (lightType == "point") light.type = Light::POINT_LIGHT;
		else if (lightType == "directional") light.type = Light::DIRECTIONAL_LIGHT;
		else if (lightType == "spot") light.type = Light::SPOT_LIGHT;
		else if (lightType == "area") light.type = Light::AREA_LIGHT;
		else if (lightType == "hemisphere") light.type = Light::HEMISPHERE_LIGHT;
		else logError("Unknown light type \"%s\"", lightType.c_str());
		setColor(light.color, lightDef["color"]);
		if (!def["position"].is_null())
			light.position = toVec3(def["position"]);
		if (!lightDef["direction"].is_null())
			light.direction = toVec3(lightDef["direction"]);
		setNumber(light.distance, lightDef["distance"]);
		setNumber(light.decay, lightDef["decay"]);
		entity.add(light);
		numLights++;
	}

	if (!def["geometry"].is_null()) {
		Model model;
		parseModel(model, def, resources);
		entity.add(model);
		numModels++;
	}

	// Patch bounding box
	// TODO: Bounding box is not correct if scale changed at runtime
	if (entity.has<Model>() && entity.has<Transform>()) {
		Model& model = entity.get<Model>();
		const Transform& trans = entity.get<Transform>();
		model.bounds.min = model.geometry->bounds.min * trans.scale;
		model.bounds.max = model.geometry->bounds.max * trans.scale;
		model.bounds.radius = model.geometry->bounds.radius * glm::compMax(trans.scale);
	}

	// Parse body (needs to be after geometry, transform, bounds...)
	if (!def["body"].is_null()) {
		const Json& bodyDef = def["body"];
		ASSERT(bodyDef.is_object());
		ASSERT(entity.has<Model>());
		ASSERT(entity.has<Transform>());
		const Model& model = entity.get<Model>();
		const Transform& transform = entity.get<Transform>();

		btCollisionShape* shape = NULL;
		const string& shapeStr = bodyDef["shape"].string_value();
		vec3 extents = model.bounds.max - model.bounds.min;
		if (shapeStr == "box") {
			shape = new btBoxShape(convert(extents * 0.5f));
		} else if (shapeStr == "sphere") {
			shape = new btSphereShape(model.bounds.radius);
		} else if (shapeStr == "cylinder") {
			shape = new btCylinderShape(convert(extents * 0.5f));
		} else if (shapeStr == "capsule") {
			float r = glm::max(extents.x, extents.z) * 0.5f;
			shape = new btCapsuleShape(r, extents.y);
		} else if (shapeStr == "trimesh") {
			if (!model.geometry->collisionMesh)
				model.geometry->generateCollisionTriMesh();
			shape = new btBvhTriangleMeshShape(model.geometry->collisionMesh, true);
		} else {
			logError("Unknown shape %s", shapeStr.c_str());
		}
		ASSERT(shape);

		float mass = 0.f;
		setNumber(mass, bodyDef["mass"]);

		btVector3 inertia(0, 0, 0);
		shape->calculateLocalInertia(mass, inertia);

		btRigidBody::btRigidBodyConstructionInfo info(mass, NULL, shape, inertia);
		info.m_startWorldTransform = btTransform(convert(transform.rotation), convert(transform.position));
		setNumber(info.m_friction, bodyDef["friction"]);
		setNumber(info.m_rollingFriction, bodyDef["rollingFriction"]);
		setNumber(info.m_restitution, bodyDef["restitution"]);
		if (bodyDef["noSleep"].bool_value()) {
			info.m_linearSleepingThreshold = 0.f;
			info.m_angularSleepingThreshold = 0.f;
		}
		entity.add<btRigidBody>(info);
		numBodies++;
		btRigidBody& body = entity.get<btRigidBody>();
		if (!bodyDef["angularFactor"].is_null())
			body.setAngularFactor(convert(toVec3(bodyDef["angularFactor"])));
		if (!bodyDef["linearFactor"].is_null())
			body.setLinearFactor(convert(toVec3(bodyDef["linearFactor"])));
		if (bodyDef["noGravity"].bool_value())
			body.setFlags(body.getFlags() | BT_DISABLE_WORLD_GRAVITY);
		body.setUserIndex(entity.get_id());
		if (world->has_system<PhysicsSystem>())
			world->get_system<PhysicsSystem>().add(entity);
	}

	if (!def["animation"].is_null()) {
		ASSERT(entity.has<Model>());
		const Json& animDef = def["animation"];
		BoneAnimation anim;
		setNumber(anim.speed, animDef["speed"]);
		entity.add(anim);
		if (animDef["play"].is_bool() && animDef["play"].bool_value())
			world->get_system<AnimationSystem>().play(entity);
		else world->get_system<AnimationSystem>().stop(entity);
	}

	if (def["trackGround"].bool_value()) {
		ASSERT(entity.has<btRigidBody>());
		entity.add<GroundTracker>();
	}

	if (def["trackContacts"].bool_value()) {
		ASSERT(entity.has<btRigidBody>());
		entity.add<ContactTracker>();
	}

	if (!def["moveSound"].is_null()) {
		const Json& soundDef = def["moveSound"];
		MoveSound sound;
		if (soundDef["event"].is_string())
			sound.event = id::hash(soundDef["event"].string_value());
		setNumber(sound.stepLength, soundDef["step"]);
		ASSERT(sound.event);
		ASSERT(entity.has<Transform>());
		sound.prevPos = entity.get<Transform>().position;
		entity.add(sound);
	}

	if (!def["contactSound"].is_null()) {
		const Json& soundDef = def["contactSound"];
		ContactSound sound;
		if (soundDef["event"].is_string())
			sound.event = id::hash(soundDef["event"].string_value());
		ASSERT(sound.event);
		ASSERT(entity.has<Transform>());
		ASSERT(entity.has<ContactTracker>());
		entity.add(sound);
	}

	return entity;
}

void SceneLoader::reset()
{
	prefabs.clear();
	numModels = 0; numBodies = 0; numLights = 0;
}

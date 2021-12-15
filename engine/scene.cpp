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
#include <glm/gtx/component_wise.hpp>

#ifndef SHIPPING_BUILD
#define USE_DEBUG_NAMES
#endif

namespace {

	// /foo/bar/baz.txt --> /foo/bar/
	string dirname(const string& path) {
		size_t pos = path.find_last_of("/");
		if (pos != string::npos)
			return path.substr(0, pos + 1);
		else return "";
	}

	string resolvePath(const string& dir, const string& path) {
		ASSERT(!path.empty());
		ASSERT(dir.empty() || dir[dir.size()-1] == '/');
		if (!path.empty() && path[0] == '/')
			return path.substr(1);
		return dir + path;
	}

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

	template<typename T> inline void setNumber(T& dst, const Json& src) {
		if (src.is_number())
			dst = (T)src.number_value();
	}

	template<typename T> inline void setEnum(T& dst, const Json& src) {
		if (src.is_number())
			dst = (T)(int)src.number_value();
	}

	template<typename T> inline void setFlag(T& dst, uint flag, const Json& src) {
		if (src.is_bool()) {
			if (src.bool_value()) dst |= flag;
			else dst &= ~flag;
		}
	}

	inline void setBool(bool& dst, const Json& src) {
		if (src.is_bool())
			dst = src.bool_value();
	}

	inline void setVec2(vec2& dst, const Json& src) {
		if (!src.is_null())
			dst = toVec2(src);
	}

	inline void setVec3(vec3& dst, const Json& src) {
		if (!src.is_null())
			dst = toVec3(src);
	}

	inline void setQuat(quat& dst, const Json& src) {
		if (src.is_array()) {
			if (src.array_items().size() == 4)
				dst = quat(src[3].number_value(), src[0].number_value(), src[1].number_value(), src[2].number_value());
			else dst = quat(toVec3(src));
		}
	}

	inline void setColor(vec3& dst, const Json& src) {
		if (!src.is_null())
			dst = colorToVec3(src);
	}

	inline void setString(string& dst, const Json& src) {
		if (src.is_string())
			dst = src.string_value();
	}

	inline void setHash(uint& dst, const Json& src) {
		if (src.is_string())
			dst = id::hash(src.string_value().c_str());
	}

	template<typename T> inline void set(T& var, const Json& src);
	template<> inline void set<float>(float& dst, const Json& src) { setNumber(dst, src); }
	template<> inline void set<vec3>(vec3& dst, const Json& src) { setVec3(dst, src); }
	template<> inline void set<quat>(quat& dst, const Json& src) { setQuat(dst, src); }

	Material& parseMaterial(Material& material, const Json& def, Resources& resources, const string& pathContext) {
		ASSERT(def.is_object());
		setString(material.shaderName, def["shaderName"]);
		setFlag(material.flags, Material::TESSELLATE, def["tessellate"]);
		setFlag(material.flags, Material::CAST_SHADOW, def["castShadow"]);
		setFlag(material.flags, Material::RECEIVE_SHADOW, def["receiveShadow"]);
		setFlag(material.flags, Material::ANIMATED, def["animated"]);
		setFlag(material.flags, Material::DRAW_REFLECTION, def["drawReflection"]);
		setColor(material.ambient, def["ambient"]);
		setColor(material.diffuse, def["diffuse"]);
		setColor(material.specular, def["specular"]);
		setColor(material.emissive, def["emissive"]);
		setNumber(material.shininess, def["shininess"]);
		setNumber(material.reflectivity, def["reflectivity"]);
		setNumber(material.parallax, def["parallax"]);
		setEnum(material.blendFunc, def["blendFunc"]);
		setNumber(material.alphaTest, def["alphaTest"]);
		if (def["alphaTest"].is_bool() && def["alphaTest"].bool_value())
			material.alphaTest = 0.9f; // Backwards compat

		setVec2(material.uvOffset, def["uvOffset"]);
		setVec2(material.uvRepeat, def["uvRepeat"]);
		setVec2(material.particleSize, def["particleSize"]);

		if (!def["diffuseMap"].is_null()) {
			material.map[Material::DIFFUSE_MAP] = resources.getImageAsync(resolvePath(pathContext, def["diffuseMap"].string_value()));
			material.map[Material::DIFFUSE_MAP]->sRGB = true;
		}
		if (!def["specularMap"].is_null()) {
			material.map[Material::SPECULAR_MAP] = resources.getImageAsync(resolvePath(pathContext, def["specularMap"].string_value()));
			material.map[Material::SPECULAR_MAP]->sRGB = true;
		}
		if (!def["emissionMap"].is_null()) {
			material.map[Material::EMISSION_MAP] = resources.getImageAsync(resolvePath(pathContext, def["emissionMap"].string_value()));
			material.map[Material::EMISSION_MAP]->sRGB = true;
		}
		if (!def["normalMap"].is_null())
			material.map[Material::NORMAL_MAP] = resources.getImageAsync(resolvePath(pathContext, def["normalMap"].string_value()));
		if (!def["heightMap"].is_null())
			material.map[Material::HEIGHT_MAP] = resources.getImageAsync(resolvePath(pathContext, def["heightMap"].string_value()));
		if (!def["aoMap"].is_null())
			material.map[Material::AO_MAP] = resources.getImageAsync(resolvePath(pathContext, def["aoMap"].string_value()));
		if (!def["reflectionMap"].is_null())
			material.map[Material::REFLECTION_MAP] = resources.getImageAsync(resolvePath(pathContext, def["reflectionMap"].string_value()));

		return material;
	}

	void parseModel(Model& model, const Json& def, Resources& resources, const string& pathContext) {
		// Parse geometry
		if (!def["geometry"].is_null()) {
			const Json& defGeom = def["geometry"];
			if (defGeom.is_string()) {
				const string geomPath = resolvePath(pathContext, defGeom.string_value());
				if (endsWith(geomPath, ".png") || endsWith(geomPath, ".jpg") || endsWith(geomPath, ".jpeg") || endsWith(geomPath, ".tga"))
					model.lods[0].geometry = resources.getHeightmap(geomPath);
				else model.lods[0].geometry = resources.getGeometry(geomPath);
			} else if (defGeom.is_array()) {
				const Json::array& lods = defGeom.array_items();
				ASSERT(lods.size() <= Model::MAX_LODS);
				int i = 0;
				for (auto& lodDef : lods) {
					ASSERT(lodDef.is_object());
					const string geomPath = resolvePath(pathContext, lodDef.object_items().begin()->first);
					model.lods[i].geometry = resources.getGeometry(geomPath);
					model.lods[i].distSq = lodDef.object_items().begin()->second.number_value();
					model.lods[i].distSq *= model.lods[i].distSq;
					++i;
				}
			} else if (defGeom.is_object()) {
				if (defGeom["particles"].is_number()) {
					// TODO: This will leak as nothing will delete it...
					model.lods[0].geometry = new Geometry((uint)defGeom["particles"].number_value());
				} else ASSERT(!"Unknown geometry definition");
			} else ASSERT(!"Unknown geometry definition");
			model.geometry = model.lods[0].geometry;
		}

		// Parse material
		const Json& materialDef = def["material"];
		if (materialDef.is_object()) {
			ASSERT(model.materials.size() <= 1);
			if (model.materials.empty())
				model.materials.emplace_back();
			parseMaterial(model.materials.back(), materialDef, resources, pathContext);
		} else if (materialDef.is_array()) {
			if (model.materials.empty()) {
				for (auto& matDef : materialDef.array_items()) {
					model.materials.emplace_back();
					parseMaterial(model.materials.back(), matDef, resources, pathContext);
				}
			} else {
				ASSERT(model.materials.size() == materialDef.array_items().size());
				for (uint i = 0; i < materialDef.array_items().size(); ++i)
					parseMaterial(model.materials[i], materialDef[i], resources, pathContext);
			}
		}
	}

	Environment parseEnvironment(const Json& def, Resources& resources, const string& pathContext)
	{
		Environment env;
		ASSERT(def.is_object());
		if (def["skybox"].is_string()) {
			env.skyType = Environment::SKY_SKYBOX;
			string skyboxPath = def["skybox"].string_value();
			if (skyboxPath.back() == '/' || skyboxPath.back() == '\\') {
				skyboxPath = resolvePath(pathContext, skyboxPath);
				env.skybox[0] = resources.getImage(skyboxPath + "px.jpg");
				env.skybox[1] = resources.getImage(skyboxPath + "nx.jpg");
				env.skybox[2] = resources.getImage(skyboxPath + "py.jpg");
				env.skybox[3] = resources.getImage(skyboxPath + "ny.jpg");
				env.skybox[4] = resources.getImage(skyboxPath + "pz.jpg");
				env.skybox[5] = resources.getImage(skyboxPath + "nz.jpg");
			} else {
				skyboxPath = resolvePath(pathContext, skyboxPath);
				for (int i = 0; i < 6; i++)
					env.skybox[i] = resources.getImage(skyboxPath);
			}
		} else if (def["skybox"].is_array()) {
			env.skyType = Environment::SKY_SKYBOX;
			for (int i = 0; i < 6; i++)
				env.skybox[i] = resources.getImage(resolvePath(pathContext, def["skybox"][i].string_value()));
		}
		else env.skyType = Environment::SKY_PROCEDURAL;
		for (int i = 0; i < 6; i++)
			if (env.skybox[i])
				env.skybox[i]->sRGB = true;

		setNumber(env.exposure, def["exposure"]);
		setNumber(env.shadowDarkness, def["shadowDarkness"]);
		setNumber(env.bloomThreshold, def["bloomThreshold"]);
		setNumber(env.bloomIntensity, def["bloomIntensity"]);
		setEnum(env.tonemap, def["tonemap"]);
		setColor(env.ambient, def["ambient"]);
		if (!def["sunPosition"].is_null())
			env.sunPosition = toVec3(def["sunPosition"]);
		setColor(env.sunColor, def["sunColor"]);
		setColor(env.fogColor, def["fogColor"]);
		setNumber(env.fogDensity, def["fogDensity"]);
		return env;
	}

	template<typename T> void addTrack(PropertyAnimation& dest, const PropertyAnimation::Track<T>& track);
	template<> void addTrack(PropertyAnimation& dest, const PropertyAnimation::Track<float>& track) { dest.floatTracks.emplace_back(track); }
	template<> void addTrack(PropertyAnimation& dest, const PropertyAnimation::Track<vec3>& track) { dest.vec3Tracks.emplace_back(track); }
	template<> void addTrack(PropertyAnimation& dest, const PropertyAnimation::Track<quat>& track) { dest.quatTracks.emplace_back(track); }

	template<typename T>
	void parsePropertyAnimationTrack(PropertyAnimation& dest, const Json& trackDef) {
		PropertyAnimation::Track<T> track(0, {});
		setHash(track.id, trackDef["id"]);
		for (const auto& keyframeDef : trackDef["keyframes"].array_items()) {
			if (keyframeDef.is_array() && keyframeDef.array_items().size() >= 2) {
				PropertyAnimation::Keyframe<T> keyframe;
				setNumber(keyframe.time, keyframeDef.array_items()[0]);
				set(keyframe.value, keyframeDef.array_items()[1]);
				track.keyframes.emplace_back(keyframe);
			}
		}
		addTrack(dest, track);
	}
}

void SceneLoader::load(const string& path, Resources& resources)
{
	logDebug("Start loading scene %s", path.c_str());
	uint t0 = Engine::timems();
	m_environment = Json();
	load_internal(path, resources);

	if (!m_environment.is_null() && world->has_system<RenderSystem>()) {
		RenderSystem& renderer = world->get_system<RenderSystem>();
		renderer.env() = parseEnvironment(m_environment, resources, dirname(path));
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
		camera.makePerspective(45.f, ar, 0.1f, 1000.f);
		if (cameraEnt.has<Transform>()) {
			Transform& trans = cameraEnt.get<Transform>();
			camera.updateViewMatrix(trans.position, trans.rotation);
		} else {
			camera.view = glm::lookAt(/*eye_pos*/ -forward_axis, /*target*/ vec3(0, 0, 0), up_axis);
		}
	}

	resources.startAsyncLoading();

	uint t1 = Engine::timems();
	logDebug("Loaded scene %s in %dms with %d models, %d bodies, %d lights, %d particles, %d prefabs", path.c_str(), t1 - t0, numModels, numBodies, numLights, numParticles, prefabs.size());
}

void SceneLoader::load_internal(const string& path, Resources& resources)
{
	string pathContext = dirname(path);
	std::string err;
	Json jsonScene = Json::parse(resources.getText(path, Resources::NO_CACHE), err);
	if (!err.empty())
		panic("Failed to read scene %s: %s", path.c_str(), err.c_str());

	if (jsonScene.is_object()) {
		// Handle includes
		if (jsonScene["include"].is_string()) {
			load_internal(resolvePath(pathContext, jsonScene["include"].string_value()), resources);
		} else if (jsonScene["include"].is_array()) {
			for (auto& includePath : jsonScene["include"].array_items())
				load_internal(resolvePath(pathContext, includePath.string_value()), resources);
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
				string path = resources.findPath(resolvePath(pathContext, it.second["path"].string_value()));
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
					audio.add(it.first, resources.getBinary(resolvePath(pathContext, it.second.string_value())));
				} else if (it.second.is_array()) {
					for (auto& it2 : it.second.array_items()) {
						ASSERT(it2.is_string());
						audio.add(it.first, resources.getBinary(resolvePath(pathContext, it2.string_value())));
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
		instantiate(objects[i], resources, pathContext);
	}
}

Entity SceneLoader::instantiate(Json def, Resources& resources, const string& pathContext)
{
	ASSERT(def.is_object());

	if (def["prefab"].is_string()) {
		const string& prefabName = def["prefab"].string_value();
		auto prefabIter = prefabs.find(prefabName);
		if (prefabIter != prefabs.end()) {
			def = assign(prefabIter->second, def);
		} else if (endsWith(prefabName, ".json")) {
				string err;
				Json extPrefab = Json::parse(resources.getText(resolvePath(pathContext, prefabName), Resources::NO_CACHE), err);
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
#ifdef USE_DEBUG_NAMES
		DebugInfo info;
		info.name = def["name"].string_value();
		entity.add<DebugInfo>(info);
	} else {
		static uint debugId = 0;
		DebugInfo info;
		if (def["prefab"].is_string())
			info.name = def["prefab"].string_value() + "#";
		else if (def["geometry"].is_string())
			info.name = def["geometry"].string_value() + "#";
		else if (def["particles"].is_object())
			info.name = "particles#";
		else info.name = "object#";
		info.name += std::to_string(debugId++);
		entity.tag(info.name);
		entity.add<DebugInfo>(info);
#endif
	}

	// Parse transform
	if (!def["position"].is_null() || !def["rotation"].is_null() || !def["scale"].is_null() || !def["geometry"].is_null()) {
		Transform transform;
		setVec3(transform.position, def["position"]);
		setVec3(transform.scale, def["scale"]);
		setQuat(transform.rotation, def["rotation"]);
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

	// Parse model
	if (!def["geometry"].is_null()) {
		Model model;
		parseModel(model, def, resources, pathContext);
		entity.add(model);
		numModels++;
	}

	// Apply bounding box to model
	// TODO: This might be an artifact of world space bounds, investigate if it should be removed
	if (entity.has<Model>() && entity.has<Transform>()) {
		Model& model = entity.get<Model>();
		model.bounds = model.lods[0].geometry->bounds;
	}

	// Parse particles
	const Json& particleDef = def["particles"];
	if (!particleDef.is_null()) {
		Particles particles;
		setNumber(particles.count, particleDef["count"]);
		setHash(particles.computeId, particleDef["compute"]);
		setVec2(particles.emitRadiusMinMax, particleDef["emitRadius"]);
		setVec2(particles.lifeTimeMinMax, particleDef["lifeTime"]);
		setVec2(particles.speedMinMax, particleDef["speed"]);
		setNumber(particles.directionality, particleDef["directionality"]);
		setNumber(particles.randomRotation, particleDef["randomRotation"]);
		setBool(particles.emit, particleDef["emit"]);
		setBool(particles.localSpace, particleDef["localSpace"]);

		const Json& materialDef = def["material"]; // Not embedded in particleDef
		if (materialDef.is_object()) {
			parseMaterial(particles.material, materialDef, resources, pathContext);
		}
		particles.bounds.radius = 10.f; // TODO: Auto-compute this from GPU
		entity.add(particles);
		numParticles++;
	}

	// Parse body (needs to be after geometry, transform, bounds...)
	if (!def["body"].is_null()) {
		const Json& bodyDef = def["body"];
		ASSERT(bodyDef.is_object());
		ASSERT(entity.has<Model>());
		ASSERT(entity.has<Transform>());
		const Model& model = entity.get<Model>();
		const Transform& transform = entity.get<Transform>();

		float mass = 0.f;
		setNumber(mass, bodyDef["mass"]);

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
			Geometry* colGeo = nullptr;
			if (bodyDef["geometry"].is_string()) {
				colGeo = resources.getGeometry(bodyDef["geometry"].string_value());
			} else {
				if (bodyDef["geometry"].is_array())
					logError("LODs not supported for collision mesh.");
				colGeo = model.lods[0].geometry;
			}
			if (!colGeo->collisionMesh)
				colGeo->generateCollisionTriMesh();
			if (mass <= 0.f) { // Static mesh
				shape = new btBvhTriangleMeshShape(colGeo->collisionMesh, true);
			} else {
				shape = new btGImpactMeshShape(colGeo->collisionMesh);
				shape->setLocalScaling(convert(transform.scale));
				static_cast<btGImpactMeshShape*>(shape)->updateBound();
			}
		} else {
			logError("Unknown shape %s", shapeStr.c_str());
		}
		ASSERT((shapeStr == "trimesh" || bodyDef["geometry"].is_null()) && "Trimesh shape type required if body.geometry is specified");
		ASSERT(shape);

		shape->setLocalScaling(convert(transform.scale));
		btVector3 inertia(0, 0, 0);
		if (mass > 0)
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
		RigidBody& rb = entity.add<RigidBody>();
		rb.body = new btRigidBody(info);
		numBodies++;
		btRigidBody& body = *rb.body;
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

	if (def["propertyAnimation"].is_object()) {
		const Json& animDef = def["propertyAnimation"];
		PropertyAnimation anim;
		setEnum(anim.mode, animDef["mode"]);
		setNumber(anim.speed, animDef["speed"]);

		const Json& tracksDef = animDef["tracks"];
		if (tracksDef.is_array()) {
			for (const auto& trackDef : tracksDef.array_items()) {
				if (trackDef["type"].is_string()) {
					const std::string& trackType = trackDef["type"].string_value();
					if (trackType == "float") {
						parsePropertyAnimationTrack<float>(anim, trackDef);
					} else if (trackType == "vec3") {
						parsePropertyAnimationTrack<vec3>(anim, trackDef);
					} else if (trackType == "quat") {
						parsePropertyAnimationTrack<quat>(anim, trackDef);
					} else logError("Invalid property animation track type \"%s\"", trackType.c_str());
				}
			}
		}
		entity.add(anim);
		if (animDef["play"].is_bool() && animDef["play"].bool_value())
			world->get_system<AnimationSystem>().play(entity);
		else world->get_system<AnimationSystem>().stop(entity);
	}

	if (def["trackGround"].bool_value()) {
		ASSERT(entity.has<RigidBody>());
		entity.add<GroundTracker>();
	}

	if (def["trackContacts"].bool_value()) {
		ASSERT(entity.has<RigidBody>());
		entity.add<ContactTracker>();
	}

	if (def["triggerVolume"].is_object()) {
		ASSERT(entity.has<Transform>());
		const Json& triggerDef = def["triggerVolume"];
		TriggerVolume& trigger = entity.add<TriggerVolume>();

		setNumber(trigger.times, triggerDef["times"]);
		setNumber(trigger.bounds.radius, triggerDef["radius"]);
		setVec3(trigger.bounds.min, triggerDef["min"]);
		setVec3(trigger.bounds.min, triggerDef["max"]);

		if (triggerDef["receiver"].is_string())
			trigger.receiverModule = id::hash(triggerDef["receiver"].string_value());
		if (triggerDef["enterMessage"].is_string())
			trigger.enterMessage = id::hash(triggerDef["enterMessage"].string_value());
		else if (triggerDef["enterMessage"].is_number())
			trigger.enterMessage = triggerDef["enterMessage"].number_value();
		if (triggerDef["exitMessage"].is_string())
			trigger.exitMessage = id::hash(triggerDef["exitMessage"].string_value());
		else if (triggerDef["exitMessage"].is_number())
			trigger.exitMessage = triggerDef["exitMessage"].number_value();

		if (triggerDef["groups"].is_number())
			trigger.groups = 1 << (uint)triggerDef["groups"].number_value();
		else if (triggerDef["groups"].is_array()) {
			for (const auto& item : triggerDef["groups"].array_items())
				trigger.groups |= 1 << (uint)item.number_value();
		}
	}

	if (def["triggerGroup"].is_number()) {
		ASSERT(entity.has<Transform>());
		entity.add<TriggerGroup>().group = 1 << (uint)def["triggerGroup"].number_value();
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

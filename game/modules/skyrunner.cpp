#include "common.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "../controller.hpp"
#include "../game.hpp"

static btTransform startPos;

EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(INIT):
		{
			Entity pl = game.entities.get_entity_by_tag("camera");
			if (!pl.is_alive() || !pl.has<btRigidBody>())
				return;

			btRigidBody& body = pl.get<btRigidBody>();
			startPos = body.getCenterOfMassTransform();
			break;
		}
		case $id(UPDATE):
		{
			Entity pl = game.entities.get_entity_by_tag("camera");
			if (!pl.is_alive() || !pl.has<btRigidBody>())
				return;

			btRigidBody& body = pl.get<btRigidBody>();
			if (body.getCenterOfMassPosition().y() < -2) {
				body.setCenterOfMassTransform(startPos);
			}
			break;
		}
		case $id(GENERATE_LEVEL):
		{
			Scene loader(game.entities);
			loader.load("skyrunner.json", game.resources);
			const Json& block = loader.prefabs["skyblock"];
			vec3 pos(0, -1, 0);
			for (int i = 0; i < 100; i++) {
				Entity e = loader.instantiate(block, game.resources);
				// TODO: Need to make this position setting easier
				if (e.has<Model>()) {
					Model& model = e.get<Model>();
					model.position = pos;
				}
				if (e.has<btRigidBody>()) {
					btRigidBody& body = e.get<btRigidBody>();
					btTransform trans(body.getCenterOfMassTransform().getRotation(), convert(pos));
					body.setWorldTransform(trans);
				}
				// Adjust position
				float xrand = glm::linearRand(-1.f, 1.f);
				if (xrand < -0.6f || xrand > 0.6f)
					pos.x += xrand;
				else pos.x = 0;
				if (glm::linearRand(0.f, 1.f) < 0.25f)
					pos.y += glm::linearRand(-0.5f, 1.2f);
				if (glm::linearRand(0.f, 1.f) < 0.2f)
					pos.z -= glm::linearRand(1.5f, 3.0f);
				else pos.z -= 1.f;
			}

			Entity cameraEnt = game.entities.get_entity_by_tag("camera");
			Camera& camera = cameraEnt.get<Camera>();
			cameraEnt.add<Controller>(camera.position, camera.rotation);
			Controller& controller = cameraEnt.get<Controller>();
			if (cameraEnt.has<btRigidBody>())
				controller.body = &cameraEnt.get<btRigidBody>();
			break;
		}
	}
}


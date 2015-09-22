#include "common.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "../controller.hpp"
#include "../game.hpp"


EXPORT void ModuleFunc(uint msg, void* param)
{
	Game& game = *static_cast<Game*>(param);
	switch (msg) {
		case $id(GENERATE_LEVEL):
		{
			Scene loader(game.entities);
			loader.load("skyrunner.json", game.resources);
			const Json& block = loader.prefabs["skyblock"];
			for (int i = 0; i < 10; i++) {
				Entity e = loader.instantiate(block, game.resources);
				// TODO: Need to make this position setting easier
				vec3 pos = vec3(0, -1, -i * 1.f);
				if (e.has<Model>()) {
					Model& model = e.get<Model>();
					model.position = pos;
				}
				if (e.has<btRigidBody>()) {
					btRigidBody& body = e.get<btRigidBody>();
					btTransform trans(body.getCenterOfMassTransform().getRotation(), convert(pos));
					body.setWorldTransform(trans);
				}
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


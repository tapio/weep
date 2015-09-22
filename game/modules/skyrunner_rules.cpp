#include "common.hpp"
#include "physics.hpp"
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
	}
}


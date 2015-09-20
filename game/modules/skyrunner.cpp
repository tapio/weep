#include "common.hpp"
#include "physics.hpp"

static btTransform startPos;

EXPORT void GameplayInit(Entities& entities)
{
	Entity pl = entities.get_entity_by_tag("camera");
	if (!pl.is_alive() || !pl.has<btRigidBody>())
		return;

	btRigidBody& body = pl.get<btRigidBody>();
	startPos = body.getCenterOfMassTransform();
}

EXPORT void GameplayUpdate(Entities& entities)
{
	Entity pl = entities.get_entity_by_tag("camera");
	if (!pl.is_alive() || !pl.has<btRigidBody>())
		return;

	btRigidBody& body = pl.get<btRigidBody>();
	if (body.getCenterOfMassPosition().y() < -2) {
		body.setCenterOfMassTransform(startPos);
	}
}


#include "common.hpp"
#include "physics.hpp"

static btTransform startPos;

EXPORT void ModuleFunc(uint msg, void* param)
{
	switch (msg) {
		case $id(INIT):
		{
			Entities& entities = *static_cast<Entities*>(param);
			Entity pl = entities.get_entity_by_tag("camera");
			if (!pl.is_alive() || !pl.has<btRigidBody>())
				return;

			btRigidBody& body = pl.get<btRigidBody>();
			startPos = body.getCenterOfMassTransform();
			break;
		}
		case $id(UPDATE):
		{
			Entities& entities = *static_cast<Entities*>(param);
			Entity pl = entities.get_entity_by_tag("camera");
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


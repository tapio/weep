#pragma once
#include "common.hpp"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/Gimpact/btGImpactShape.h"
#include <ecs/ecs.hpp>

struct RigidBody;

class PhysicsSystem : public ecs::System
{
public:
	PhysicsSystem();
	~PhysicsSystem();
	void reset();

	bool add(ecs::Entity entity);
	void destroy(ecs::Entity entity) override;

	void step(ecs::Entities& entities, float dt, bool fixedStep);

	ecs::Entity rayCast(ecs::Entities& entities, vec3 from, vec3 to);
	bool testGroundHit(RigidBody& body);

	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btBroadphaseInterface* broadphase;
	btCollisionDispatcher* dispatcher;
	btConstraintSolver*	solver;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btDiscreteDynamicsWorld* dynamicsWorld;
};

vec3 inline convert(const btVector3& vector) {
	return vec3(vector.x(), vector.y(), vector.z());
}

btVector3 inline convert(const vec3& vector) {
	return btVector3(vector.x, vector.y, vector.z);
}

quat inline convert(const btQuaternion& q) {
	return quat(q.w(), q.x(), q.y(), q.z());
}

btQuaternion inline convert(const quat& q) {
	return btQuaternion(q.x, q.y, q.z, q.w);
}

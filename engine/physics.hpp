#pragma once
#include "common.hpp"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

class PhysicsSystem : public System
{
public:
	PhysicsSystem();
	~PhysicsSystem();
	void reset();

	bool add(Entity entity);

	void step(Entities& entities, float dt);

	bool testGroundHit(btRigidBody& body);

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

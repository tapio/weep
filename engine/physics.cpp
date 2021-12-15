#include "physics.hpp"
#include "components.hpp"
#include "geometry.hpp"
#include "scene.hpp"
#include "engine.hpp"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/btBulletDynamicsCommon.h"

PhysicsSystem::PhysicsSystem()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
}

PhysicsSystem::~PhysicsSystem()
{
	reset();
	delete dynamicsWorld;
	delete solver;
	delete broadphase;
	delete dispatcher;
	delete collisionConfiguration;
}

void PhysicsSystem::reset()
{
	ASSERT(dynamicsWorld);
	for (int i = dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
	{
		dynamicsWorld->removeConstraint(dynamicsWorld->getConstraint(i));
	}
	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
	}
	for (int i = 0; i < collisionShapes.size(); i++)
	{
		btCollisionShape* shape = collisionShapes[i];
		delete shape;
	}
	collisionShapes.clear();
}

void PhysicsSystem::step(Entities& entities, float dt, bool fixedStep)
{
	// Update manual transform changes to physics
	entities.for_each<btRigidBody, Transform>([](Entity, btRigidBody& body, Transform& transform) {
		if (transform.dirty) {
			btTransform trans(convert(transform.rotation), convert(transform.position));
			body.setCenterOfMassTransform(trans);
		}
	});
	entities.for_each<ContactTracker>([&](Entity, ContactTracker& tracker) {
		tracker.hadContact = false;
	});

	// Simulate
	ASSERT(dynamicsWorld);
	int maxSteps = fixedStep ? 5 : 0;
	dynamicsWorld->stepSimulation(dt, maxSteps);

	// Sync physics results to entity transforms
	entities.for_each<btRigidBody, Transform>([](Entity, btRigidBody& body, Transform& transform) {
		const btTransform& trans = body.getCenterOfMassTransform();
		transform.position = convert(trans.getOrigin());
		transform.rotation = convert(trans.getRotation());
	});

	// ContactTracker
	int numManifolds = dispatcher->getNumManifolds();
	for (int i = 0;i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold =  dispatcher->getManifoldByIndexInternal(i);
		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();
		int numContacts = contactManifold->getNumContacts();
		if (numContacts) {
			Entity entityA(objA->getUserIndex(), &entities);
			Entity entityB(objB->getUserIndex(), &entities);
			// TODO: Figure out a way to filter what contacts to tracks
			if (entityA.has<ContactTracker>() && entityB.has<ContactTracker>()) {
				entityA.get<ContactTracker>().hadContact = true;
				entityB.get<ContactTracker>().hadContact = true;
			}
		}
		/*for (int j = 0;j < numContacts; j++) {
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.f) {
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;
			}
		}*/
	}

	// GroundTracker
	entities.for_each<GroundTracker, btRigidBody>([&](Entity, GroundTracker& tracker, btRigidBody& body) {
		tracker.onGround = testGroundHit(body);
	});
}

Entity PhysicsSystem::rayCast(Entities& entities, vec3 from_, vec3 to_)
{
	btVector3 from = convert(from_);
	btVector3 to = convert(to_);
	btCollisionWorld::ClosestRayResultCallback res(from, to);
	dynamicsWorld->rayTest(from, to, res);
	if (res.hasHit()) {
		return Entity(res.m_collisionObject->getUserIndex(), &entities);
	}
	return {};
}

bool PhysicsSystem::testGroundHit(btRigidBody& body)
{
	btVector3 from = body.getCenterOfMassPosition();
	btVector3 to(from.x(), from.y() - 10.0, from.z());
	btCollisionWorld::ClosestRayResultCallback res(from, to);
	dynamicsWorld->rayTest(from, to, res);
	if (res.hasHit()) {
		btVector3 aabbMin, aabbMax;
		body.getAabb(aabbMin, aabbMax);
		btScalar d = (aabbMax.y() - aabbMin.y()) * 0.5f + 0.01f;
		return res.m_hitPointWorld.distance2(from) <= d * d;
	}
	return false;
}

bool PhysicsSystem::add(Entity entity)
{
	if (!entity.has<btRigidBody>()) return false;
	btRigidBody& body = entity.get<btRigidBody>();
	ASSERT(!body.isInWorld());
	collisionShapes.push_back(body.getCollisionShape());
	dynamicsWorld->addRigidBody(&body);
	return true;
}

void PhysicsSystem::destroy(Entity entity)
{
	if (!entity.has<btRigidBody>()) return;
	btRigidBody& body = entity.get<btRigidBody>();
	ASSERT(body.isInWorld());
	if (body.getMotionState())
		delete body.getMotionState();
	dynamicsWorld->removeRigidBody(&body);
	ASSERT(body.getCollisionShape());
	collisionShapes.remove(body.getCollisionShape());
	delete body.getCollisionShape();
}

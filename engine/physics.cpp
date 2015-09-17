#include "physics.hpp"
#include "model.hpp"
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
	dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
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

void PhysicsSystem::step(float dt)
{
	ASSERT(dynamicsWorld);
	dynamicsWorld->stepSimulation(dt);
}

void PhysicsSystem::syncTransforms(Scene& scene)
{
	scene.world.for_each<Model, btRigidBody>([](Entity, Model& model, btRigidBody& body) {
		const btTransform& trans = body.getCenterOfMassTransform();
		model.position = convert(trans.getOrigin());
		model.rotation = convert(trans.getRotation());
	});
}

bool PhysicsSystem::addModel(Entity entity)
{
	if (!entity.has<btRigidBody>()) return false;
	btRigidBody& body = entity.get<btRigidBody>();
	ASSERT(!body.isInWorld());
	collisionShapes.push_back(body.getCollisionShape());
	dynamicsWorld->addRigidBody(&body);
	return true;
}

void PhysicsSystem::addScene(Scene& scene)
{
	uint t0 = Engine::timems();
	int count = 0;
	scene.world.for_each<btRigidBody>([this, &count](Entity e, btRigidBody) {
		count += addModel(e);
	});
	uint t1 = Engine::timems();
	logDebug("Loaded %d bodies in %dms", count, t1 - t0);
}

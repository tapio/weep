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
		delete obj;
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
	for (auto& model : scene.getChildren()) {
		if (model.body) {
			const btTransform& trans = model.body->getCenterOfMassTransform();
			model.position = convert(trans.getOrigin());
			model.rotation = convert(trans.getRotation());
		}
	}
}

bool PhysicsSystem::addModel(Model& model)
{
	Model::BodyDef& def = model.bodyDef;
	if (def.shape == Model::BodyDef::SHAPE_NONE)
		return false;

	btCollisionShape* shape = NULL;
	if (def.shape == Model::BodyDef::SHAPE_BOX) {
		BoundingBox aabb = model.geometry->boundingBox;
		shape = new btBoxShape(convert((aabb.max - aabb.min) * 0.5f));
	} else if (def.shape == Model::BodyDef::SHAPE_SPHERE) {
		shape = new btSphereShape(model.geometry->boundingRadius);
	}
	collisionShapes.push_back(shape);

	btRigidBody::btRigidBodyConstructionInfo info(def.mass, NULL, shape);
	btRigidBody* body = new btRigidBody(info);
	btTransform trans(convert(model.rotation), convert(model.position));
	body->setWorldTransform(trans);
	body->setUserPointer(&model);
	dynamicsWorld->addRigidBody(body);
	model.body = body;
	return true;
}

void PhysicsSystem::addScene(Scene& scene)
{
	uint t0 = Engine::timems();
	int count = 0;
	for (auto& it : scene.getChildren()) {
		count += addModel(it);
	}
	uint t1 = Engine::timems();
	logDebug("Loaded %d bodies in %dms", count, t1 - t0);
}

#include "physics.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "scene.hpp"
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
	for (int i = 0; i < collisionShapes.size(); i++)
	{
		btCollisionShape* shape = collisionShapes[i];
		delete shape;
	}
	collisionShapes.clear();

	delete dynamicsWorld;
	delete solver;
	delete broadphase;
	delete dispatcher;
	delete collisionConfiguration;
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

void PhysicsSystem::addModel(Model& model)
{
	Model::BodyDef& def = model.bodyDef;
	if (def.shape == Model::BodyDef::SHAPE_NONE)
		return;

	btCollisionShape* shape = NULL;
	if (def.shape == Model::BodyDef::SHAPE_BOX) {
		BoundingBox aabb = model.geometry->boundingBox;
		shape = new btBoxShape(convert((aabb.max - aabb.min) * 0.5f));
	} else if (def.shape == Model::BodyDef::SHAPE_SPHERE) {
		shape = new btSphereShape(model.geometry->boundingRadius);
	}

	btRigidBody::btRigidBodyConstructionInfo info(def.mass, NULL, shape);
	btRigidBody* body = new btRigidBody(info);
	btTransform trans(convert(model.rotation), convert(model.position));
	body->setWorldTransform(trans);
	body->setUserPointer(&model);
	dynamicsWorld->addRigidBody(body);
	model.body = body;
}

void PhysicsSystem::addScene(Scene& scene)
{
	for (auto& it : scene.getChildren()) {
		addModel(it);
	}
}

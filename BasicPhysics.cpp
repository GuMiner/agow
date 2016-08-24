#include <limits>
#include "Math\MatrixOps.h"
#include "Math\VecOps.h"
#include "Utils\Logger.h"
#include "BasicPhysics.h"

BasicPhysics::BasicPhysics()
{
}

void BasicPhysics::LoadBasicCollisionShapes()
{
	// TODO configurable
	float height = 0.75;
	float width = 0.50;

    CollisionShapes.clear();
	CollisionShapes[CShape::NPC_CAPSULE] = new btCapsuleShape(width / 2.0f, height); // TODO other NPC shapes.
	CollisionShapes[CShape::NPC_CUBOID] = new btBoxShape(btVector3(width, width, height));
	
    const int diamondPointCount = 8;
	vec::vec3* diamondPoints = new vec::vec3[diamondPointCount];
	diamondPoints[0] = vec::vec3(0.0f, 0.0f, -height);
	diamondPoints[1] = vec::vec3(width / 2.0f, 0.0f, 0.0f);
	diamondPoints[2] = vec::vec3(-width / 2.0f, 0.0f, 0.0f);
	diamondPoints[3] = vec::vec3(0.0f, width / 2.0f, 0.0f);
	diamondPoints[4] = vec::vec3(0.0f, -width / 2.0f, 0.0f);
	diamondPoints[5] = vec::vec3(0.0f, 0.0f, height);

    diamondPoints[6] = vec::vec3(-width / 2.0f, 0.0f, -height);
    diamondPoints[7] = vec::vec3(0.0f, width / 2.0f, -height);

	CollisionShapes[CShape::NPC_DIAMOND] = new btConvexHullShape((btScalar*)&diamondPoints[0], diamondPointCount, sizeof(vec::vec3));
	delete[] diamondPoints;

    CollisionShapes[CShape::SMALL_CUBE] = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
    
    const int playerPointCount = 4;
    vec::vec3* playerPoints = new vec::vec3[playerPointCount];
    playerPoints[0] = vec::vec3(0.0f, 0.0f, 0.7825f);
    playerPoints[1] = vec::vec3(-0.50f, 0.2887f, -0.4423f);
    playerPoints[2] = vec::vec3(0.50f, 0.2887f, -0.4423f);
    playerPoints[3] = vec::vec3(0.0f, 0.0f, 0.4825f);
   
    CollisionShapes[CShape::PLAYER] = new btConvexHullShape((btScalar*)&playerPoints[0], playerPointCount, sizeof(vec::vec3));
    delete[] playerPoints;
}

bool BasicPhysics::LoadPhysics()
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
    broadphaseCollisionDetector = new btDbvtBroadphase();
    constraintSolver = new btSequentialImpulseConstraintSolver();
    
    DynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphaseCollisionDetector,
        constraintSolver, collisionConfiguration);

    DynamicsWorld->setGravity(btVector3(0, 0, -9.80f));
    LoadBasicCollisionShapes();

    return true;
}

void BasicPhysics::Step(float timestep)
{
    // Honestly this doesn't need to be its own method, but it encapsulates this nicely.
    DynamicsWorld->stepSimulation(timestep, 10);
}

void BasicPhysics::UnloadPhysics()
{
    // Delete the collision shapes.
    for (auto iter = CollisionShapes.begin(); iter != CollisionShapes.end(); iter++)
    {
        delete iter->second;
    }

    // Delete basic setup of physics
    delete DynamicsWorld;

    delete constraintSolver;
    delete broadphaseCollisionDetector;
    delete collisionDispatcher;
    delete collisionConfiguration;
}

btRigidBody* BasicPhysics::GetStaticBody(const CShape shape, const btVector3& origin)
{
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(origin);

    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo ground(0.0f, motionState, CollisionShapes[shape]);
    return new btRigidBody(ground);
}

btRigidBody* BasicPhysics::GetDynamicBody(const CShape shape, const btVector3& origin, const float mass)
{
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(origin);

    btVector3 localInertia;
    CollisionShapes[shape]->calculateLocalInertia(mass, localInertia);
    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo object(mass, motionState, CollisionShapes[shape], localInertia);
    return new btRigidBody(object);
}

vec::vec3 BasicPhysics::GetBodyPosition(const btRigidBody* body)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    btVector3& pos = worldTransform.getOrigin();
    return vec::vec3(pos.x(), pos.y(), pos.z());
}

void BasicPhysics::Warp(btRigidBody* body, vec::vec3 position, vec::vec3 velocity)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);
    worldTransform.setIdentity();
    worldTransform.setOrigin(btVector3(position.x, position.y, position.z));

    body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    body->activate(true);
    body->setWorldTransform(worldTransform);
    body->getMotionState()->setWorldTransform(worldTransform);
}

vec::mat4 BasicPhysics::GetBodyMatrix(const btRigidBody* body)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    vec::quaternion rotation = VecOps::Convert(worldTransform.getRotation());
    return MatrixOps::Translate(VecOps::Convert(worldTransform.getOrigin())) * rotation.asMatrix();
}

vec::quaternion BasicPhysics::GetBodyRotation(const btRigidBody* body)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    return VecOps::Convert(worldTransform.getRotation());
}

void BasicPhysics::DeleteBody(btRigidBody* body) const
{
    delete body->getMotionState();
    delete body;
}
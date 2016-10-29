#include <limits>
#include <glm\gtc\quaternion.hpp>
#include "Data\UserPhysics.h"
#include "Math\PhysicsOps.h"
#include "Utils\Logger.h"
#include "Utils\TypedCallback.h"
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
    glm::vec3* diamondPoints = new glm::vec3[diamondPointCount];
    diamondPoints[0] = glm::vec3(0.0f, 0.0f, -height);
    diamondPoints[1] = glm::vec3(width / 2.0f, 0.0f, 0.0f);
    diamondPoints[2] = glm::vec3(-width / 2.0f, 0.0f, 0.0f);
    diamondPoints[3] = glm::vec3(0.0f, width / 2.0f, 0.0f);
    diamondPoints[4] = glm::vec3(0.0f, -width / 2.0f, 0.0f);
    diamondPoints[5] = glm::vec3(0.0f, 0.0f, height);

    diamondPoints[6] = glm::vec3(-width / 2.0f, 0.0f, -height);
    diamondPoints[7] = glm::vec3(0.0f, width / 2.0f, -height);

    CollisionShapes[CShape::NPC_DIAMOND] = new btConvexHullShape((btScalar*)&diamondPoints[0], diamondPointCount, sizeof(glm::vec3));
    delete[] diamondPoints;

    // TODO these all should be configurable size values (TODO configurable).
    CollisionShapes[CShape::NPC_NEARFIELD_BUBBLE] = new btSphereShape(height * 4);

    CollisionShapes[CShape::SMALL_CUBE] = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
    
    CollisionShapes[CShape::WEAPON_PLASMA] = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

    const int playerPointCount = 4;
    glm::vec3* playerPoints = new glm::vec3[playerPointCount];
    playerPoints[0] = glm::vec3(0.0f, 0.0f, 0.7825f);
    playerPoints[1] = glm::vec3(-0.50f, 0.2887f, -0.4423f);
    playerPoints[2] = glm::vec3(0.50f, 0.2887f, -0.4423f);
    playerPoints[3] = glm::vec3(0.0f, 0.0f, 0.4825f);
   
    CollisionShapes[CShape::PLAYER] = new btConvexHullShape((btScalar*)&playerPoints[0], playerPointCount, sizeof(glm::vec3));
    delete[] playerPoints;
}

bool BasicPhysics::LoadPhysics(btIDebugDraw* debugDrawer)
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
    broadphaseCollisionDetector = new btDbvtBroadphase();
    constraintSolver = new btSequentialImpulseConstraintSolver();
    
    DynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphaseCollisionDetector,
        constraintSolver, collisionConfiguration);

    DynamicsWorld->setGravity(btVector3(0, 0, -9.80f));
    DynamicsWorld->setDebugDrawer(debugDrawer);
    
    btContactSolverInfo& solverInfo = DynamicsWorld->getSolverInfo();
    solverInfo.m_numIterations = 10;

    // Our basic collision shapes are hardcoded, and any model-based shapes are passed-in directly.
    LoadBasicCollisionShapes();

    return true;
}

// Adds collsion models to the list of known collision shapes.
void BasicPhysics::AddCollisionModels(std::map<CShape, const std::vector<glm::vec3>*> shapePoints)
{
    for (std::pair<const CShape, const std::vector<glm::vec3>*> shapePointPair : shapePoints)
    {
        CollisionShapes[shapePointPair.first] = new btConvexHullShape((btScalar*)&(*shapePointPair.second)[0], shapePointPair.second->size(), sizeof(glm::vec3));
    }
}

void BasicPhysics::Step(float timestep)
{
    // Run our simulation!
    DynamicsWorld->stepSimulation(timestep);

    // Check for interesting collisions.
    const int manifoldCount = collisionDispatcher->getNumManifolds();
    for (int i = 0; i < manifoldCount; i++)
    {
        const btPersistentManifold* manifold = collisionDispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* objOne = manifold->getBody0();
        const btCollisionObject* objTwo = manifold->getBody1();

        void* userObj1 = objOne->getUserPointer();
        void* userObj2 = objTwo->getUserPointer();
        if (userObj1 != nullptr && userObj2 != nullptr)
        {
            // These objects could collide, so verify this is a real collision.
            bool doesCollide = false;
            int contactCount = manifold->getNumContacts();
            for (int j = 0; j < contactCount; j++)
            {
                if (manifold->getContactPoint(j).getDistance() < 0.0f)
                {
                    doesCollide = true;
                    break;
                }
            }

            if (doesCollide)
            {
                // These are two objects we know about, so decipher their types and call the callbacks for collisions as appropriate.
                TypedCallback<UserPhysics::ObjectType>* userObject1 = (TypedCallback<UserPhysics::ObjectType>*)userObj1;
                TypedCallback<UserPhysics::ObjectType>* userObject2 = (TypedCallback<UserPhysics::ObjectType>*)userObj2;
                if (UserPhysics::Collides(userObject1->GetType(), userObject2->GetType()))
                {
                    userObject1->CallCallback(userObject2->GetType());
                }

                if (UserPhysics::Collides(userObject2->GetType(), userObject1->GetType()))
                {
                    userObject2->CallCallback(userObject1->GetType());
                }
            }
        }
    }
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
    btRigidBody::btRigidBodyConstructionInfo bodyInfo(0.0f, motionState, CollisionShapes[shape]);
    btRigidBody* body = new btRigidBody(bodyInfo);
    body->setUserPointer(nullptr);
    return body;
}

btRigidBody* BasicPhysics::GetStaticBody(btCollisionShape* collisionShape, const btVector3& origin)
{
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(origin);

    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo bodyInfo(0.0f, motionState, collisionShape);
    btRigidBody* body = new btRigidBody(bodyInfo);
    body->setUserPointer(nullptr);
    return body;
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
    btRigidBody* newBody = new btRigidBody(object);
    newBody->setFriction(0.50f); // TODO configurable.
    newBody->setUserPointer(nullptr);
    return newBody;
}

btRigidBody* BasicPhysics::GetDynamicBody(btCollisionShape* collisionShape, const btVector3& origin, const float mass)
{
    btTransform pos;
    pos.setIdentity();
    pos.setOrigin(origin);

    btVector3 localInertia;
    collisionShape->calculateLocalInertia(mass, localInertia);
    btDefaultMotionState *motionState = new btDefaultMotionState(pos);
    btRigidBody::btRigidBodyConstructionInfo object(mass, motionState, collisionShape, localInertia);
    btRigidBody* newBody = new btRigidBody(object);
    newBody->setFriction(0.50f); // TODO configurable.
    newBody->setUserPointer(nullptr);
    return newBody;
}

btRigidBody* BasicPhysics::GetGhostObject(btCollisionShape* collisionShape, const btVector3& origin)
{
    btRigidBody* rigidBody = GetDynamicBody(collisionShape, origin, 1.0f);
    rigidBody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
    return rigidBody;
}

btRigidBody* BasicPhysics::GetGhostObject(const CShape shape, const btVector3& origin)
{
    btRigidBody* rigidBody = GetDynamicBody(shape, origin, 1.0f);
    rigidBody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
    return rigidBody;
}

glm::vec3 BasicPhysics::GetBodyPosition(const btRigidBody* body)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    btVector3& pos = worldTransform.getOrigin();
    return glm::vec3(pos.x(), pos.y(), pos.z());
}

void BasicPhysics::Warp(btRigidBody* body, glm::vec3 position, glm::vec3 velocity)
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

glm::mat4 BasicPhysics::GetBodyMatrix(const btRigidBody* body)
{
    glm::mat4 result;
    body->getWorldTransform().getOpenGLMatrix((btScalar*)&result);
    return result;
    
    // glm::quat rotation = VecOps::Convert(worldTransform.getRotation());
    // return MatrixOps::Translate(VecOps::Convert(worldTransform.getOrigin())) * rotation.asMatrix();
}

glm::quat BasicPhysics::GetBodyRotation(const btRigidBody* body)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    btQuaternion worldRotation = worldTransform.getRotation();
    glm::quat quat;
    quat.x = worldRotation.x();
    quat.y = worldRotation.y();
    quat.z = worldRotation.z();
    quat.w = worldRotation.w();
    return quat;
}

void BasicPhysics::DeleteGhostObject(btRigidBody* ghostObject) const
{
    DeleteBody(ghostObject, false);
}

void BasicPhysics::DeleteBody(btRigidBody* body, bool eraseCollisionShape) const
{
    if (eraseCollisionShape)
    {
        delete body->getCollisionShape();
    }

    if (body->getUserPointer() != nullptr)
    {
        delete body->getUserPointer();
    }

    delete body->getMotionState();
    delete body;
}
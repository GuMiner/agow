#include <limits>
#include <glm\gtc\quaternion.hpp>
#include "Data\UserPhysics.h"
#include "Generators\PhysicsGenerator.h"
#include "Math\PhysicsOps.h"
#include "Utils\Logger.h"
#include "Utils\TypedCallback.h"
#include "Physics.h"

Physics::Physics()
    : queuedCommands(), accumulatedTimestep(0.0f), simulating(false)
{
}

bool Physics::LoadPhysics(PhysicsDebugDrawer* debugDrawer)
{
    this->debugDrawer = debugDrawer;
    collisionConfiguration = new btDefaultCollisionConfiguration();
    collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
    broadphaseCollisionDetector = new btDbvtBroadphase();
    constraintSolver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphaseCollisionDetector,
        constraintSolver, collisionConfiguration);

    dynamicsWorld->setGravity(btVector3(0, 0, -9.80f));
    dynamicsWorld->setDebugDrawer(debugDrawer);
    
    btContactSolverInfo& solverInfo = dynamicsWorld->getSolverInfo();
    solverInfo.m_numIterations = 10;

    // Our basic collision shapes are hardcoded, and any model-based shapes are passed-in directly.
    PhysicsGenerator::LoadCollisionShapes();

    return true;
}

void Physics::Step(float timestep)
{
    accumulatedTimestep += timestep;
    if (simulating)
    {
        if (simulationThread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            simulating = false;
            PerformQueuedActions();
            PerformPostStepActions();
        }
    }
    
    if (!simulating)
    {
        // Run our simulation!
        simulationThread = std::async(std::launch::async, &Physics::PerformStep, this, accumulatedTimestep);
        accumulatedTimestep = 0;
        simulating = true;
    }
}

void Physics::PerformStep(float timestep)
{
    // Honestly this could be in a lambda instead.
    dynamicsWorld->stepSimulation(timestep);
}

void Physics::PerformQueuedActions()
{
    removedBodies.clear();

    for (unsigned int i = 0; i < queuedCommands.size(); i++)
    {
        switch (queuedCommands[i].action)
        {
        case PhysicsCommand::AddBody:
            dynamicsWorld->addRigidBody((btRigidBody*)queuedCommands[i].item);
            break;
        case PhysicsCommand::RemoveBody:
            dynamicsWorld->removeRigidBody((btRigidBody*)queuedCommands[i].item);
            removedBodies.insert(queuedCommands[i].item);
            break;
        case PhysicsCommand::DeleteBody:
            if (((btRigidBody*)queuedCommands[i].item)->getUserPointer() != nullptr)
            {
                delete ((btRigidBody*)queuedCommands[i].item)->getUserPointer();
            }

            delete ((btRigidBody*)queuedCommands[i].item)->getMotionState();
            delete queuedCommands[i].item;
            removedBodies.insert(queuedCommands[i].item);
            break;
        case PhysicsCommand::DeleteBodyAndCollisionShapes:
            delete ((btRigidBody*)queuedCommands[i].item)->getCollisionShape();

            if (((btRigidBody*)queuedCommands[i].item)->getUserPointer() != nullptr)
            {
                delete ((btRigidBody*)queuedCommands[i].item)->getUserPointer();
            }

            delete ((btRigidBody*)queuedCommands[i].item)->getMotionState();
            delete queuedCommands[i].item;
            removedBodies.insert(queuedCommands[i].item);
            break;
        default:
            break;
        }
    }

    queuedCommands.clear();
}

void Physics::PerformPostStepActions()
{
    if (debugDrawer->ShouldRender())
    {
        debugDrawer->Reset();
        dynamicsWorld->debugDrawWorld();
    }

    // Check for interesting collisions.
    const int manifoldCount = collisionDispatcher->getNumManifolds();
    for (int i = 0; i < manifoldCount; i++)
    {
        const btPersistentManifold* manifold = collisionDispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* objOne = manifold->getBody0();
        const btCollisionObject* objTwo = manifold->getBody1();

        if (removedBodies.find((void*)objOne) != removedBodies.end() || removedBodies.find((void*)objTwo) != removedBodies.end())
        {
            // We removed this item in the middle of the simulation, so we don't call callbacks on it.
            continue;
        }

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

void Physics::UnloadPhysics()
{
    PhysicsGenerator::UnloadCollisionShapes();

    // Delete basic setup of physics
    delete dynamicsWorld;
    delete constraintSolver;
    delete broadphaseCollisionDetector;
    delete collisionDispatcher;
    delete collisionConfiguration;
}

void Physics::AddBody(btRigidBody* body)
{
    queuedCommands.push_back(PhysicsCommand(PhysicsCommand::AddBody, body));
}

void Physics::RemoveBody(btRigidBody* body)
{
    queuedCommands.push_back(PhysicsCommand(PhysicsCommand::RemoveBody, body));
}

void Physics::DeleteBody(btRigidBody* body, bool eraseCollisionShape)
{
    queuedCommands.push_back(PhysicsCommand(
        eraseCollisionShape ? PhysicsCommand::DeleteBodyAndCollisionShapes : PhysicsCommand::DeleteBody,
        body));
}
#include <limits>
#include <glm\gtc\quaternion.hpp>
#include "Data\UserPhysics.h"
#include "Generators\PhysicsGenerator.h"
#include "Math\PhysicsOps.h"
#include "Utils\Logger.h"
#include "Utils\TypedCallback.h"
#include "Physics.h"

std::vector<ContactCallback> Physics::contactCallbacks = std::vector<ContactCallback>();
std::map<void*, std::set<void*>> Physics::contactCallbacksFound = std::map<void*, std::set<void*>>();

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

    gContactProcessedCallback = &Physics::AddContactCallback;

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
            PerformPostStepActions();
            PerformQueuedActions();
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

bool Physics::AddContactCallback(btManifoldPoint& cp, void* body0, void* body1)
{
    if (cp.getDistance() >= 0.0f)
    {
        // This is a pre-emptive contact point -- the objects haven't collided yet.
        return false;
    }

    if (contactCallbacksFound.find(body0) == contactCallbacksFound.end())
    {
        contactCallbacksFound[body0] = std::set<void*>();
    }

    if (contactCallbacksFound[body0].find(body1) != contactCallbacksFound[body0].end())
    {
        // This item already exists.
        return false;
    }

    contactCallbacksFound[body0].insert(body1);
    contactCallbacks.push_back(ContactCallback(body0, body1));
    return true;
}

void Physics::PerformStep(float timestep)
{
    // Honestly this could be in a lambda instead.
    dynamicsWorld->stepSimulation(timestep, 4);
}

void Physics::PerformQueuedActions()
{
    for (unsigned int i = 0; i < queuedCommands.size(); i++)
    {
        switch (queuedCommands[i].action)
        {
        case PhysicsCommand::AddBody:
            dynamicsWorld->addRigidBody((btRigidBody*)queuedCommands[i].item);
            break;
        case PhysicsCommand::RemoveBody:
            dynamicsWorld->removeRigidBody((btRigidBody*)queuedCommands[i].item);
            break;
        case PhysicsCommand::DeleteBody:
            if (((btRigidBody*)queuedCommands[i].item)->getUserPointer() != nullptr)
            {
                delete ((btRigidBody*)queuedCommands[i].item)->getUserPointer();
            }
            
            delete ((btRigidBody*)queuedCommands[i].item)->getMotionState();
            // delete queuedCommands[i].item;
            break;
        case PhysicsCommand::DeleteBodyAndCollisionShapes:
            delete ((btRigidBody*)queuedCommands[i].item)->getCollisionShape();
            
            if (((btRigidBody*)queuedCommands[i].item)->getUserPointer() != nullptr)
            {
                delete ((btRigidBody*)queuedCommands[i].item)->getUserPointer();
            }
            
            delete ((btRigidBody*)queuedCommands[i].item)->getMotionState();
            // delete queuedCommands[i].item;
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

    // Figure out what will be updated so we don't perform callbacks inadvertently on it.
    std::set<void*> removedBodies;
    for (unsigned int i = 0; i < queuedCommands.size(); i++)
    {
        switch (queuedCommands[i].action)
        {
        case PhysicsCommand::DeleteBody:
        case PhysicsCommand::DeleteBodyAndCollisionShapes:
        case PhysicsCommand::RemoveBody:
            removedBodies.insert(queuedCommands[i].item);
            break;
        default:
            break;
        }
    }

    for(const ContactCallback& callback : contactCallbacks)
    {
        if (removedBodies.find(callback.body0) != removedBodies.end() || removedBodies.find(callback.body1) != removedBodies.end())
        {
            // We removed this item in the middle of the simulation, so we don't call callbacks on it.
            continue;
        }

        const btCollisionObject* objOne = (btCollisionObject*)callback.body0;
        const btCollisionObject* objTwo = (btCollisionObject*)callback.body1;

        void* userObj1 = objOne->getUserPointer();
        void* userObj2 = objTwo->getUserPointer();
        if (userObj1 != nullptr && userObj2 != nullptr)
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

    contactCallbacks.clear();
    contactCallbacksFound.clear();
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
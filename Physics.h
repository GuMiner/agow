#pragma once
#include <future>
#include <set>
#include <vector>
#include <Bullet\btBulletDynamicsCommon.h>
#include "PhysicsDebugDrawer.h"

struct PhysicsCommand
{
    enum Action
    {
        AddBody = 0,
        RemoveBody = 1,
        DeleteBody = 2,
        DeleteBodyAndCollisionShapes = 3
    };

    Action action;
    void* item;

    PhysicsCommand(Action action, void* item)
        : action(action), item(item)
    {
    }
};

// Defines the basics of physics (ie, gravity) the rest of the game uses.
// Also holds generic framework code.
class Physics
{
    std::set<void*> removedBodies;
    std::vector<PhysicsCommand> queuedCommands;

    float accumulatedTimestep;
    bool simulating;
    std::future<void> simulationThread;

    // Bullet Physics
    btCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *collisionDispatcher;
    btBroadphaseInterface *broadphaseCollisionDetector;
    btConstraintSolver *constraintSolver;
    btDiscreteDynamicsWorld *dynamicsWorld;

    PhysicsDebugDrawer* debugDrawer;

    void PerformStep(float timestep); // Runs the physics simulation on a separate thread.
    void PerformQueuedActions(); // Performs any queued physics actions that could not be done in multiple threads.
    void PerformPostStepActions(); // Performs physics that occurs after a step occurs.

public:

    Physics();
    bool LoadPhysics(PhysicsDebugDrawer* debugDrawer);
    void Step(float timestep);
    void UnloadPhysics();

    void AddBody(btRigidBody* body);
    void RemoveBody(btRigidBody* body);
    void DeleteBody(btRigidBody* body, bool deleteCollisionShape);
};


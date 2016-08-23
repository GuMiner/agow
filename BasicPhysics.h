#pragma once
#include <map>
#include <Bullet\btBulletDynamicsCommon.h>
#include "Math\Vec.h"

// Defines the basics of physics (ie, gravity) the rest of the game uses.
// Also holds generic framework code.
class BasicPhysics
{
    // Bullet Physics
    btCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *collisionDispatcher;
    btBroadphaseInterface *broadphaseCollisionDetector;
    btConstraintSolver *constraintSolver;

    void LoadBasicCollisionShapes();

public:
    enum CShape 
    {
        SMALL_CUBE,
        NPC_CAPSULE,
		NPC_CUBOID,
		NPC_DIAMOND,
        PLAYER
    };

    std::map<CShape, btCollisionShape*> CollisionShapes;
    btDiscreteDynamicsWorld *DynamicsWorld;

    BasicPhysics();
    bool LoadPhysics();
    void Step(float timestep);
    void UnloadPhysics();

    // Simplifies returning a physics shape from preset settings. Ensure you call delete if you call get!
    btRigidBody* GetStaticBody(const CShape shape, const btVector3& origin);
    btRigidBody* GetDynamicBody(const CShape shape, const btVector3& origin, const float mass);
    
    // Gets the body position, converting to our coordinate system.
    static vec::vec3 GetBodyPosition(const btRigidBody* body);
    
    // Instantaneously moves the item to a new position.
    static void Warp(btRigidBody* body, vec::vec3 position, vec::vec3 velocity);

    // Converts from Bullet geometry to our game geometry.
    static vec::mat4 GetBodyMatrix(const btRigidBody* body);
    static vec::quaternion GetBodyRotation(const btRigidBody* body);
    void DeleteBody(btRigidBody* body) const;
};


#pragma once
#include <map>
#include <vector>
#include <Bullet\btBulletDynamicsCommon.h>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp> 

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
        NPC_NEARFIELD_BUBBLE,
        ROCK_OCTAHEDRON,
        ROCK_DODECAHEDRON,
        ROCK_ICOSAHEDRON,
        ROCK_OCTOTOAD,
        ROCK_SPHERE_ONE,
        ROCK_SPHERE_TWO,
        ROCK_SPHERE_THREE,
        ROCK_TETRAHEDRON,
        WEAPON_PLASMA,
        SIGN_STOP,
        PLAYER
    };

    std::map<CShape, btCollisionShape*> CollisionShapes;
    btDiscreteDynamicsWorld *DynamicsWorld;

    BasicPhysics();
    bool LoadPhysics(btIDebugDraw* debugDrawer);
    void AddCollisionModels(std::map<CShape, const std::vector<glm::vec3>*> shapePoints);
    void Step(float timestep);
    void UnloadPhysics();

    // Simplifies returning a physics shape from preset settings. Ensure you call delete if you call get!
    btRigidBody* GetStaticBody(const CShape shape, const btVector3& origin);
    btRigidBody* GetDynamicBody(const CShape shape, const btVector3& origin, const float mass);

    // Same as the above, but with a non-standard collision shape. Calling delete can automatically remove the collision shape.
    btRigidBody* GetStaticBody(btCollisionShape* collisionShape,const btVector3& origin);
    btRigidBody* GetDynamicBody(btCollisionShape* collisionShape,const btVector3& origin, const float mass);

    // An actual rigid body, but with collision interaction disabled.
    btRigidBody* GetGhostObject(btCollisionShape* collisionShape, const btVector3& origin);
    btRigidBody* GetGhostObject(const CShape shape, const btVector3& origin);
    
    // Gets the body position, converting to our coordinate system.
    static glm::vec3 GetBodyPosition(const btRigidBody* body);
    
    // Instantaneously moves the item to a new position.
    static void Warp(btRigidBody* body, glm::vec3 position, glm::vec3 velocity);

    // Converts from Bullet geometry to our game geometry.
    static glm::mat4 GetBodyMatrix(const btRigidBody* body);
    static glm::quat GetBodyRotation(const btRigidBody* body);
    void DeleteBody(btRigidBody* body, bool deleteCollisionShape) const;
    void DeleteGhostObject(btRigidBody* ghostObject) const;
};


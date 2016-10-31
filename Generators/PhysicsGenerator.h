#pragma once
#include <map>
#include <vector>
#include <Bullet\btBulletDynamicsCommon.h>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp> 

class PhysicsGenerator
{
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

private:
    static std::map<CShape, btCollisionShape*> CollisionShapes;
    
public:
    static void LoadCollisionShapes();
    static void AddCollisionModels(std::map<CShape, const std::vector<glm::vec3>*> shapePoints);

    // Simplifies returning a physics shape from preset settings. Ensure you call delete if you call get!
    static btRigidBody* GetStaticBody(const CShape shape, const btVector3& origin);
    static btRigidBody* GetDynamicBody(const CShape shape, const btVector3& origin, const float mass);

    // Same as the above, but with a non-standard collision shape. Calling delete can automatically remove the collision shape.
    static btRigidBody* GetStaticBody(btCollisionShape* collisionShape, const btVector3& origin);
    static btRigidBody* GetDynamicBody(btCollisionShape* collisionShape, const btVector3& origin, const float mass);

    // An actual rigid body, but with collision interaction disabled.
    static btRigidBody* GetGhostObject(btCollisionShape* collisionShape, const btVector3& origin);
    static btRigidBody* GetGhostObject(const CShape shape, const btVector3& origin);

    // TODO these aren't really 'generation' and should be elsewhere.

    // Gets the body position, converting to our coordinate system.
    static glm::vec3 GetBodyPosition(const btRigidBody* body);

    // Converts from Bullet geometry to our game geometry.
    static glm::mat4 GetBodyMatrix(const btRigidBody* body);
    static glm::quat GetBodyRotation(const btRigidBody* body);

    static void UnloadCollisionShapes();
};


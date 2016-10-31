#include "PhysicsGenerator.h"

std::map<PhysicsGenerator::CShape, btCollisionShape*> PhysicsGenerator::CollisionShapes;

void PhysicsGenerator::LoadCollisionShapes()
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

// Adds collsion models to the list of known collision shapes.
void PhysicsGenerator::AddCollisionModels(std::map<CShape, const std::vector<glm::vec3>*> shapePoints)
{
    for (std::pair<const CShape, const std::vector<glm::vec3>*> shapePointPair : shapePoints)
    {
        CollisionShapes[shapePointPair.first] = new btConvexHullShape((btScalar*)&(*shapePointPair.second)[0], shapePointPair.second->size(), sizeof(glm::vec3));
    }
}

btRigidBody* PhysicsGenerator::GetStaticBody(const CShape shape, const btVector3& origin)
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

btRigidBody* PhysicsGenerator::GetStaticBody(btCollisionShape* collisionShape, const btVector3& origin)
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

btRigidBody* PhysicsGenerator::GetDynamicBody(const CShape shape, const btVector3& origin, const float mass)
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

btRigidBody* PhysicsGenerator::GetDynamicBody(btCollisionShape* collisionShape, const btVector3& origin, const float mass)
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

btRigidBody* PhysicsGenerator::GetGhostObject(btCollisionShape* collisionShape, const btVector3& origin)
{
    btRigidBody* rigidBody = GetDynamicBody(collisionShape, origin, 1.0f);
    rigidBody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
    return rigidBody;
}

btRigidBody* PhysicsGenerator::GetGhostObject(const CShape shape, const btVector3& origin)
{
    btRigidBody* rigidBody = GetDynamicBody(shape, origin, 1.0f);
    rigidBody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
    return rigidBody;
}

glm::vec3 PhysicsGenerator::GetBodyPosition(const btRigidBody* body)
{
    btTransform worldTransform;
    body->getMotionState()->getWorldTransform(worldTransform);

    btVector3& pos = worldTransform.getOrigin();
    return glm::vec3(pos.x(), pos.y(), pos.z());
}

glm::mat4 PhysicsGenerator::GetBodyMatrix(const btRigidBody* body)
{
    // Slower, alternate route.
    // glm::quat rotation = VecOps::Convert(worldTransform.getRotation());
    // return MatrixOps::Translate(VecOps::Convert(worldTransform.getOrigin())) * rotation.asMatrix();

    glm::mat4 result;
    body->getWorldTransform().getOpenGLMatrix((btScalar*)&result);
    return result;
}

glm::quat PhysicsGenerator::GetBodyRotation(const btRigidBody* body)
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

void PhysicsGenerator::UnloadCollisionShapes()
{
    // Delete the collision shapes.
    for (auto iter = CollisionShapes.begin(); iter != CollisionShapes.end(); iter++)
    {
        delete iter->second;
    }
}
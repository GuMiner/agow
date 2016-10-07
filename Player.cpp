#include <iostream>
#include <glm\gtc\quaternion.hpp>
#include "Config\PhysicsConfig.h"
#include "Data\TerrainTile.h"
#include "Math\PhysicsOps.h"
#include "Input.h"
#include "Map.h"
#include "Player.h"

Player::Player()
    : lastMousePos(glm::ivec2(-1, -1)), camera(75, glm::vec2(-30, 30), glm::vec2(-14, 14)), isOnGround(false), motionType(ON_FOOT), // TODO configurable camera.
      enemyKos(0), allyKos(0), civilianKos(0)
{
}

bool Player::LoadPlayerModel(ModelManager* modelManager)
{
    physicalModel.modelId = modelManager->LoadModel("models/player");
    return physicalModel.modelId != 0;
}

void Player::LoadPlayerPhysics(BasicPhysics physics, glm::vec3 startingPosition, float mass)
{
    physicalModel.rigidBody = physics.GetDynamicBody(BasicPhysics::CShape::PLAYER, PhysicsOps::Convert(startingPosition), mass);
    physicalModel.rigidBody->forceActivationState(DISABLE_DEACTIVATION);
    physicalModel.rigidBody->setAngularFactor(0.0f);
    physicalModel.rigidBody->setFriction(2.0f); // TODO configurable.
    physicalModel.rigidBody->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::PLAYER, this));
    physics.DynamicsWorld->addRigidBody(physicalModel.rigidBody);

    camera.Initialize(physicalModel.rigidBody);
}

void Player::UnloadPlayerPhysics(BasicPhysics physics)
{
    delete physicalModel.rigidBody->getUserPointer();
    physics.DynamicsWorld->removeRigidBody(physicalModel.rigidBody);
    physics.DeleteBody(physicalModel.rigidBody, false);
}

void Player::Callback(UserPhysics::ObjectType collidingObject, void* callbackSpecificData)
{
    if (collidingObject == UserPhysics::ObjectType::HEIGHTMAP)
    {
        isOnGround = true;
    }
}

const glm::vec2 Player::GetTerrainPosition() const
{
    glm::vec3 bodyPos = BasicPhysics::GetBodyPosition(physicalModel.rigidBody);
    return glm::vec2(bodyPos.x, bodyPos.y);
}

const glm::vec3 Player::GetViewPosition() const
{
    return camera.GetViewPosition();
}

const glm::quat Player::GetViewOrientation() const
{
    return camera.GetViewOrientation();
}

const glm::mat4 Player::GetViewMatrix() const
{
    return camera.GetViewMatrix();
}

void Player::Warp(RegionManager* regionManager, btDynamicsWorld* world, const glm::vec2 mapPos)
{
    // TODO make these offsets configurable.
    float height = regionManager->GetPointHeight(world, mapPos);

    btTransform& worldTransform = physicalModel.rigidBody->getWorldTransform();
    
    worldTransform.setIdentity();
    worldTransform.setOrigin(btVector3(mapPos.x, mapPos.y, height + 4));
    physicalModel.rigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    physicalModel.rigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    physicalModel.rigidBody->activate(true); // Ensure we don't hang in midair.
    isOnGround = false;
}

void Player::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
    glm::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(physicalModel.rigidBody);
    modelManager->RenderModel(projectionMatrix, physicalModel.modelId, mvMatrix, false);
}

void Player::Update(float frameTime, int terrainTypeOn)
{
    glm::quat travelRotation = glm::conjugate(BasicPhysics::GetBodyRotation(physicalModel.rigidBody)) * glm::rotate(glm::quat(), glm::radians(90.0f), glm::vec3(0, 0, 1));

    glm::vec3 upVector = PhysicsOps::UpVector(travelRotation);
    glm::vec3 forwardsVector = PhysicsOps::ForwardsVector(travelRotation);
    glm::vec3 sidewaysVector = glm::cross(upVector, forwardsVector);

    // TODO configurable.
    sidewaysVector = glm::normalize(sidewaysVector);
    if (Input::IsKeyPressed(GLFW_KEY_W))
    {
        physicalModel.rigidBody->applyCentralForce(PhysicsOps::Convert(sidewaysVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    if (Input::IsKeyPressed(GLFW_KEY_S))
    {
        physicalModel.rigidBody->applyCentralForce(PhysicsOps::Convert(-sidewaysVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    forwardsVector.z = 0; // Moving forwards doesn't move you down in the Z-direction.
    if (forwardsVector.x < 0.01f && forwardsVector.x > -0.01f)
    {
        forwardsVector.x = 0.01f; // Tiny nudge to avoid div/zero issues.
    }

    forwardsVector = glm::normalize(forwardsVector);

    if (Input::IsKeyPressed(GLFW_KEY_A))
    {
        btTransform& worldTransform = physicalModel.rigidBody->getWorldTransform();
        btQuaternion rotation = worldTransform.getRotation();
        rotation = rotation * btQuaternion(btVector3(0, 0, 1), 0.05f); // TODO configurable
        worldTransform.setRotation(rotation);
    }

    if (Input::IsKeyPressed(GLFW_KEY_D))
    {
        btTransform& worldTransform = physicalModel.rigidBody->getWorldTransform();
        btQuaternion rotation = worldTransform.getRotation();
        rotation = rotation * btQuaternion(btVector3(0, 0, 1), -0.05f); // TODO configurable
        worldTransform.setRotation(rotation);
    }

    // TODO configurable (jump and force),

    // Jump
    if (isOnGround && Input::IsKeyPressed(GLFW_KEY_SPACE))
    {
        physicalModel.rigidBody->applyCentralImpulse(PhysicsOps::Convert(glm::vec3(0.0f, 0.0f, 200.0f)));
        isOnGround = false;
    }

    // Camera off-center offset.
    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (lastMousePos.x == -1)
        {
            lastMousePos = Input::GetMousePos();
        }
        else
        {
            glm::ivec2 newMousePos = Input::GetMousePos();
            glm::ivec2 deltaPos = lastMousePos - newMousePos;
            lastMousePos = newMousePos;

            // Rotate the camera for x motion and y motion.
            float xAmount = PhysicsConfig::ViewRotateAroundFactor * (float)deltaPos.x;
            float yAmount = PhysicsConfig::ViewRotateUpFactor * (float)deltaPos.y;

            // Rotate around
            camera.Yaw(xAmount);
            
            // Rotate up
            camera.Pitch(yAmount);
        }
    }
    else
    {
        lastMousePos = glm::ivec2(-1, -1);
    }

    // Limit player motion.
    float speedLimitSqd = terrainTypeOn == TerrainTypes::ROADS ?
        (Player::RoadSpeedLimit * Player::RoadSpeedLimit) : (Player::SpeedLimit * Player::SpeedLimit);

    if (physicalModel.rigidBody->getLinearVelocity().length2() > speedLimitSqd)
    {
        btVector3 linearVelocity = physicalModel.rigidBody->getLinearVelocity();
        physicalModel.rigidBody->setLinearVelocity(Player::SpeedLimit * linearVelocity.normalize());
    }

    camera.Update(frameTime);
}

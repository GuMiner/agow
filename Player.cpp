#include <iostream>
#include <glm\vec2.hpp>
#include <glm\gtc\quaternion.hpp>
#include "Config\PhysicsConfig.h"
#include "Data\TerrainTile.h"
#include "Math\PhysicsOps.h"
#include "Input.h"
#include "Map.h"
#include "Player.h"

Player::Player(ModelManager* modelManager, BasicPhysics* physics)
    : gravityWeapon(physics), pressureWeapon(physics, glm::vec2(1.0f, 10.0f)), rockWeapon(modelManager, physics, glm::vec2(10.0f, 500.0f)), sunbeamWeapon(physics), // TODO configurable
      lastMousePos(glm::ivec2(-1, -1)), camera(-80, glm::vec2(-30, 30), glm::vec2(-14, 14)), isOnGround(true), motionType(ON_FOOT), // TODO configurable camera.
      enemyKos(0), allyKos(0), civilianKos(0), model()
{
    selectedWeapon = &rockWeapon;
}

bool Player::LoadPlayerModel(ModelManager* modelManager)
{
    model.modelId = modelManager->LoadModel("models/player");
    return model.modelId != 0;
}

void Player::LoadPlayerPhysics(BasicPhysics physics, glm::vec3 startingPosition, float mass)
{
    model.body = physics.GetDynamicBody(BasicPhysics::CShape::PLAYER, PhysicsOps::Convert(startingPosition), mass);
    model.body->forceActivationState(DISABLE_DEACTIVATION);
    
    // Rotate to face the forwards direction.
    btTransform& worldTransform = model.body->getWorldTransform();
    model.body->setAngularFactor(0.0f);
    model.body->setFriction(2.0f); // TODO configurable.
    model.body->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::PLAYER, this));
    physics.DynamicsWorld->addRigidBody(model.body);

    camera.Initialize(model.body);
}

void Player::UnloadPlayerPhysics(BasicPhysics physics)
{
    // TODO cleanup the weapons.

    delete model.body->getUserPointer();
    physics.DynamicsWorld->removeRigidBody(model.body);
    physics.DeleteBody(model.body, false);
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
    glm::vec3 bodyPos = BasicPhysics::GetBodyPosition(model.body);
    return glm::vec2(bodyPos.x, bodyPos.y);
}

const glm::vec3 Player::GetPosition() const
{
    return BasicPhysics::GetBodyPosition(model.body);
}

const glm::quat Player::GetOrientation() const
{
    return BasicPhysics::GetBodyRotation(model.body) * glm::rotate(glm::quat(), glm::radians(-90.0f), glm::vec3(1, 0, 0));
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

    btTransform& worldTransform = model.body->getWorldTransform();
    
    worldTransform.setIdentity();
    worldTransform.setOrigin(btVector3(mapPos.x, mapPos.y, height + 4));
    model.body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    model.body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    model.body->activate(true); // Ensure we don't hang in midair.
    isOnGround = false;
}

void Player::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
    modelManager->RenderModelImmediate(projectionMatrix, &model);

    selectedWeapon->Render(projectionMatrix);
}

void Player::Update(float frameTime, int terrainTypeOn)
{
    // Move the player around.
    glm::quat orientation = GetOrientation();
    glm::vec3 upVector = PhysicsOps::UpVector(orientation);
    glm::vec3 forwardsVector = PhysicsOps::ForwardsVector(orientation);
    glm::vec3 sidewaysVector = glm::cross(upVector, forwardsVector);

    // TODO configurable.
    forwardsVector = glm::normalize(forwardsVector);
    if (Input::IsKeyPressed(GLFW_KEY_W))
    {
        model.body->applyCentralForce(PhysicsOps::Convert(forwardsVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    if (Input::IsKeyPressed(GLFW_KEY_S))
    {
        model.body->applyCentralForce(PhysicsOps::Convert(-forwardsVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    if (Input::IsKeyPressed(GLFW_KEY_A))
    {
        btTransform& worldTransform = model.body->getWorldTransform();
        btQuaternion rotation = worldTransform.getRotation();
        rotation = rotation * btQuaternion(btVector3(0, 0, 1), 0.05f); // TODO configurable
        worldTransform.setRotation(rotation);
    }

    if (Input::IsKeyPressed(GLFW_KEY_D))
    {
        btTransform& worldTransform = model.body->getWorldTransform();
        btQuaternion rotation = worldTransform.getRotation();
        rotation = rotation * btQuaternion(btVector3(0, 0, 1), -0.05f); // TODO configurable
        worldTransform.setRotation(rotation);
    }

    // TODO configurable (jump and force),

    // Jump
    if (isOnGround && Input::IsKeyPressed(GLFW_KEY_SPACE))
    {
        model.body->applyCentralImpulse(PhysicsOps::Convert(glm::vec3(0.0f, 0.0f, 200.0f)));
        //isOnGround = false;
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

    if (model.body->getLinearVelocity().length2() > speedLimitSqd)
    {
        btVector3 linearVelocity = model.body->getLinearVelocity();
        model.body->setLinearVelocity(Player::SpeedLimit * linearVelocity.normalize());
    }

    camera.Update(frameTime);

    selectedWeapon->Update(frameTime);

    // Fire.
    // TODO configurable.
    if (Input::IsKeyTyped(GLFW_KEY_F))
    {
        // Fire a cube for collision tests.
        glm::vec3 forwardsVector = PhysicsOps::ForwardsVector(orientation);
        glm::vec3 pos = GetPosition() + 5.0f * forwardsVector;
        selectedWeapon->Fire(pos, forwardsVector);
    }
}

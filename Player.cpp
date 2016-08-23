#include "Config\PhysicsConfig.h"
#include "Math\VecOps.h"
#include "Map.h"
#include "Player.h"

Player::Player()
    : lastMousePos(sf::Vector2i(-1, -1))
{
}

bool Player::LoadPlayerModel(ModelManager* modelManager)
{
    physicalModel.modelId = modelManager->LoadModel("models/player");
    return physicalModel.modelId != 0;
}

void Player::LoadPlayerPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass)
{
    physicalModel.rigidBody = physics.GetDynamicBody(BasicPhysics::CShape::PLAYER, VecOps::Convert(startingPosition), mass);
    physicalModel.rigidBody->forceActivationState(DISABLE_DEACTIVATION);
    physicalModel.rigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 1.0f));
    physics.DynamicsWorld->addRigidBody(physicalModel.rigidBody);
}

void Player::UnloadPlayerPhysics(BasicPhysics physics)
{
    physics.DynamicsWorld->removeRigidBody(physicalModel.rigidBody);
    physics.DeleteBody(physicalModel.rigidBody);
}


const vec::vec3 Player::GetViewPosition() const
{
    vec::vec3 pos = BasicPhysics::GetBodyPosition(physicalModel.rigidBody);
    vec::quaternion orientation = GetViewOrientation();

    vec::vec3 forwardsVector = orientation.forwardVector();
    vec::vec3 upVector = orientation.upVector();
    
    // TODO configurable.
    // The view position is behind the player.
    return pos - (upVector * 1.0f + forwardsVector * 3.0f);
}

const vec::quaternion Player::GetViewOrientation() const
{
    // Our camera is rotated 90 degrees to be visible. TODO configurable
    return vec::quaternion::fromAxisAngle(3.14159f * 0.20f, vec::vec3(1, 0, 0)) * BasicPhysics::GetBodyRotation(physicalModel.rigidBody);
}

void Player::Warp(RegionManager* regionManager, btDynamicsWorld* world, const vec::vec2 mapPos)
{
    // TODO make these offsets configurable.
    float height = regionManager->GetPointHeight(world, mapPos);

    btTransform worldTransform;
    physicalModel.rigidBody->getMotionState()->getWorldTransform(worldTransform);
    
    worldTransform.setIdentity();
    worldTransform.setOrigin(btVector3(mapPos.x, mapPos.y, height + 4));
    physicalModel.rigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    physicalModel.rigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    physicalModel.rigidBody->activate(true); // Ensure we don't hang in midair.
    physicalModel.rigidBody->setWorldTransform(worldTransform);
    physicalModel.rigidBody->getMotionState()->setWorldTransform(worldTransform);
}

void Player::Render(ModelManager* modelManager, const vec::mat4& projectionMatrix)
{
    vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(physicalModel.rigidBody);
    modelManager->RenderModel(projectionMatrix, physicalModel.modelId, mvMatrix, false);
}

void Player::InputUpdate(float frameTime)
{
    // TODO make motion time-sensitive, or rendering fixed to 60 fps.

    vec::vec3 upVector = GetViewOrientation().upVector();
    vec::vec3 forwardsVector = GetViewOrientation().forwardVector();
    vec::vec3 sidewaysVector = VecOps::Cross(upVector, forwardsVector);
    if (sf::Keyboard::isKeyPressed(KeyBindingConfig::MoveLeft))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(-sidewaysVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    if (sf::Keyboard::isKeyPressed(KeyBindingConfig::MoveRight))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(sidewaysVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    forwardsVector.z = 0; // Moving forwards doesn't move you down in the Z-direction.
    if (forwardsVector.x < 0.01f && forwardsVector.x > -0.01f)
    {
        forwardsVector.x = 0.01f; // Tiny nudge to avoid div/zero issues.
    }

    forwardsVector = vec::normalize(forwardsVector);

    if (sf::Keyboard::isKeyPressed(KeyBindingConfig::MoveForward))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(forwardsVector * PhysicsConfig::ViewForwardsSpeed));
    }

    if (sf::Keyboard::isKeyPressed(KeyBindingConfig::MoveBackward))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(-forwardsVector * PhysicsConfig::ViewForwardsSpeed));
    }

    if (sf::Keyboard::isKeyPressed(KeyBindingConfig::MoveUp))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(-upVector * PhysicsConfig::ViewForwardsSpeed));
    }

    if (sf::Keyboard::isKeyPressed(KeyBindingConfig::MoveDown))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(upVector * PhysicsConfig::ViewForwardsSpeed));
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        if (lastMousePos.x == -1)
        {
            lastMousePos = sf::Mouse::getPosition();
        }
        else
        {
            sf::Vector2i newMousePos = sf::Mouse::getPosition();
            sf::Vector2i deltaPos = lastMousePos - newMousePos;
            lastMousePos = newMousePos;

            // Rotate the camera for x motion and y motion.
            float xAmount = PhysicsConfig::ViewRotateAroundFactor * (float)deltaPos.x;
            float yAmount = PhysicsConfig::ViewRotateUpFactor * (float)deltaPos.y;

            // Rotate around
            physicalModel.rigidBody->applyTorque(VecOps::Convert(upVector * xAmount));
            
            // Rotate up
            physicalModel.rigidBody->applyTorque(VecOps::Convert(sidewaysVector * yAmount));
        }
    }
    else
    {
        lastMousePos = sf::Vector2i(-1, -1);
    }
}

#include "Config\PhysicsConfig.h"
#include "Math\MathOps.h"
#include "Math\VecOps.h"
#include "Map.h"
#include "Player.h"

Player::Player()
    : lastMousePos(sf::Vector2i(-1, -1)), camera(75, vec::vec2(-30, 30), vec::vec2(-14, 14)), isOnGround(false) // TODO configurable camera.
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
    physics.DeleteBody(physicalModel.rigidBody);
}

void Player::Callback(UserPhysics::ObjectType collidingObject)
{
    if (collidingObject == UserPhysics::ObjectType::HEIGHTMAP)
    {
        isOnGround = true;
    }
}

const vec::vec2 Player::GetTerrainPosition() const
{
    vec::vec3 bodyPos = BasicPhysics::GetBodyPosition(physicalModel.rigidBody);
    return vec::vec2(bodyPos.x, bodyPos.y);
}

const vec::vec3 Player::GetViewPosition() const
{
    return camera.GetViewPosition();
}

const vec::quaternion Player::GetViewOrientation() const
{
    return camera.GetViewOrientation();
}

void Player::Warp(RegionManager* regionManager, btDynamicsWorld* world, const vec::vec2 mapPos)
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

void Player::Render(ModelManager* modelManager, const vec::mat4& projectionMatrix)
{
    vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(physicalModel.rigidBody);
    modelManager->RenderModel(projectionMatrix, physicalModel.modelId, mvMatrix, false);
}

void Player::Update(float frameTime)
{
    vec::quaternion travelRotation = BasicPhysics::GetBodyRotation(physicalModel.rigidBody).conjugate() * vec::quaternion::fromAxisAngle(MathOps::Radians(90), vec::vec3(0, 0, 1));

    vec::vec3 upVector = travelRotation.upVector();
    vec::vec3 forwardsVector = travelRotation.forwardVector();
    vec::vec3 sidewaysVector = VecOps::Cross(upVector, forwardsVector);

    sidewaysVector = vec::normalize(sidewaysVector);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(sidewaysVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        physicalModel.rigidBody->applyCentralForce(VecOps::Convert(-sidewaysVector * PhysicsConfig::ViewSidewaysSpeed));
    }

    forwardsVector.z = 0; // Moving forwards doesn't move you down in the Z-direction.
    if (forwardsVector.x < 0.01f && forwardsVector.x > -0.01f)
    {
        forwardsVector.x = 0.01f; // Tiny nudge to avoid div/zero issues.
    }

    forwardsVector = vec::normalize(forwardsVector);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        btTransform& worldTransform = physicalModel.rigidBody->getWorldTransform();
        btQuaternion rotation = worldTransform.getRotation();
        rotation = rotation * btQuaternion(btVector3(0, 0, 1), 0.05f); // TODO configurable
        worldTransform.setRotation(rotation);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        btTransform& worldTransform = physicalModel.rigidBody->getWorldTransform();
        btQuaternion rotation = worldTransform.getRotation();
        rotation = rotation * btQuaternion(btVector3(0, 0, 1), -0.05f); // TODO configurable
        worldTransform.setRotation(rotation);
    }

    // TODO configurable (jump and force),
    if (isOnGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        physicalModel.rigidBody->applyCentralImpulse(VecOps::Convert(vec::vec3(0.0f, 0.0f, 200.0f)));
        isOnGround = false;
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
            camera.Yaw(xAmount);
            
            // Rotate up
            camera.Pitch(yAmount);
        }
    }
    else
    {
        lastMousePos = sf::Vector2i(-1, -1);
    }

    camera.Update(frameTime);
}

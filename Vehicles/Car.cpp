#include "Utils\Logger.h"
#include "Car.h"

btCollisionShape* Car::vehicleFrameCollisionShape;
btCollisionShape* Car::vehicleWheelCollisionShape;
unsigned int Car::frameModelId;
unsigned int Car::wheelModelId;

bool Car::LoadModels(ModelManager* modelManager)
{
    frameModelId = modelManager->LoadModel("models/vehicles/car/frame");
    if (frameModelId == 0)
    {
        Logger::LogError("Could not load the car model!");
        return false;
    }

    wheelModelId = modelManager->LoadModel("models/vehicles/car/wheel");
    if (wheelModelId == 0)
    {
        Logger::LogError("Could not load the wheel model!");
        return false;
    }

    // TODO destruct at the end of the game.
    vehicleFrameCollisionShape = new btBoxShape(btVector3(2.0f, 1.0f, 1.0f));
    vehicleWheelCollisionShape = new btCylinderShape(btVector3(0.50, 0.25f, 0.50f));
    return true;
}

bool Car::LoadVehicleComponents(ModelManager* modelManager)
{
    return false;
}

void Car::SetupPhysics(BasicPhysics* physics)
{
    // TODO the factory should take in the location the car should spawn, along with its orientation.
    // TODO these should be constants elsewhere.
    // TODO there should be a vehicle factory, something to manage the vehicles (manager), and the individual vehicle (this).
    const btVector3 vehicleOrigin = btVector3(0.0f, 0.0f, 0.0f) + btVector3(offset.x, offset.y, offset.z);
    const float vehicleMass = 1000.0f;
    
    const float frameLength = 2.0f;
    const float frameWidth = 1.0f;
    const float frameHeight = 1.0f;
    const float wheelRadius = 0.50f;
    const float suspensionDist = 0.25f;
    
    btVector3 wheelOffsets[4] =
    {
        btVector3(frameLength / 2.0f, frameWidth / 2.0f, -(frameHeight / 2.0f + wheelRadius + suspensionDist)),
        btVector3(frameLength / 2.0f, -frameWidth / 2.0f, -(frameHeight / 2.0f + wheelRadius + suspensionDist)),
        btVector3(-frameLength / 2.0f, frameWidth / 2.0f, -(frameHeight / 2.0f + wheelRadius + suspensionDist)),
        btVector3(-frameLength / 2.0f, -frameWidth / 2.0f, -(frameHeight / 2.0f + wheelRadius + suspensionDist))
    };

    car.frame = Model();
    car.frame.modelId = frameModelId;
    car.frame.body = physics->GetDynamicBody(vehicleFrameCollisionShape, vehicleOrigin, vehicleMass);
    car.frame.body->setActivationState(DISABLE_DEACTIVATION);
    physics->DynamicsWorld->addRigidBody(car.frame.body);

    btVector3 parentAxis(0.0f, 0.0f, 1.0f);
    btVector3 childAxis(0.0f, 1.0f, 0.0f);
    for (unsigned int i = 0; i < Car::WheelCount; i++)
    {
        car.wheels[i] = Model();
        car.wheels[i].modelId = wheelModelId;
        car.wheels[i].body = physics->GetDynamicBody(vehicleWheelCollisionShape, wheelOffsets[i] + vehicleOrigin, 30);
        car.wheels[i].body->setActivationState(DISABLE_DEACTIVATION);
        car.wheels[i].body->setFriction(1250);
        physics->DynamicsWorld->addRigidBody(car.wheels[i].body);

        car.wheelConstraints[i] = 
            new btHinge2Constraint(*car.frame.body, *car.wheels[i].body, wheelOffsets[0], parentAxis, childAxis);
        car.wheelConstraints[i]->setLowerLimit(-SIMD_HALF_PI * 0.5f);
        car.wheelConstraints[i]->setUpperLimit(SIMD_HALF_PI * 0.5f);

        // Motor simulating suspension.
        int motorAxis = 3; // X-axis, angular.
        car.wheelConstraints[i]->enableMotor(motorAxis, true);
        car.wheelConstraints[i]->setMaxMotorForce(motorAxis, 1000);
        car.wheelConstraints[i]->setTargetVelocity(motorAxis, -1);

        // Motor which tries to turn the wheels back to the aligned state.
        motorAxis = 5; // Z-axis, angular.
        car.wheelConstraints[i]->enableMotor(motorAxis, true);
        car.wheelConstraints[i]->setMaxMotorForce(motorAxis, 1000);
        car.wheelConstraints[i]->setTargetVelocity(motorAxis, 0);

        physics->DynamicsWorld->addConstraint(car.wheelConstraints[i], true);
    }
}

void Car::Update(float gameTime, float frameTime)
{
}

void Car::UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide)
{
}

void Car::Render(ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
    modelManager->RenderModelImmediate(projectionMatrix, &car.frame);
    for (unsigned int i = 0; i < Car::WheelCount; i++)
    {
        modelManager->RenderModelImmediate(projectionMatrix, &car.wheels[i]);
    }
}

glm::vec3 Car::GetInteractionPoint() const
{
    return glm::vec3();
}

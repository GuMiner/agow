#pragma once
#include <glm\vec3.hpp>
#include <Bullet\btBulletDynamicsCommon.h>
#include "BasicPhysics.h"
#include "Vehicle.h"

struct IndividualCar
{
    Model frame;
    Model wheels[4];

    btHinge2Constraint* wheelConstraints[4];
    btPoint2PointConstraint* wheelConnectors[4];
};

class Car : public Vehicle
{
    static btCollisionShape* vehicleFrameCollisionShape;
    static btCollisionShape* vehicleWheelCollisionShape;
    static unsigned int frameModelId;
    static unsigned int wheelModelId;

    const unsigned int WheelCount = 4;
    IndividualCar car;

public:
    void SetupPhysics(BasicPhysics* physics);

    static bool LoadModels(ModelManager* modelManager);

    glm::vec3 offset;

    // Inherited via Vehicle
    virtual bool LoadVehicleComponents(ModelManager * modelManager) override;
    virtual void Update(float gameTime, float frameTime) override;
    virtual void UpdateInputs(bool forwards, bool backwards, bool left, bool right, bool up, bool down, bool slide) override;
    virtual void Render(ModelManager * modelManager, const glm::mat4 & projectionMatrix) override;
    virtual glm::vec3 GetInteractionPoint() const override;
};
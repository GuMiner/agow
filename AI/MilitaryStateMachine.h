#pragma once
#include <functional>
#include <glm\vec3.hpp>

class MilitaryStateMachine
{
public:
    enum State
    {
        FireAtNearest,
        FireAtTargetted,
        TakeCover,
        Flee,
        ThrowExplosive,
        TravelToFlank,
        TravelToProtect,
        Follow,
        Survey,
        CallForBackup
    };

private:
    State currentState;
    float timeInState;
    
    std::function<bool(glm::vec3*)> isNearbyEnemy;
    std::function<bool(glm::vec3*)> isTargettedEnemy;

public:
    MilitaryStateMachine();
    std::string GetStateDescription() const;
    void Update(float elapsedTime);
};
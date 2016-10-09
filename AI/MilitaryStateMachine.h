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
    State state;
    float timeInState;
    
    glm::vec3 target;

    
    // Takes in the target, returns true if the target has been reached.
    std::function<bool()> hasMovedToTarget; 
    std::function<bool()> hasThrownExplosive;

    // Returns true if an enemy is found, with the target position.
    std::function<bool(glm::vec3*)> isNearbyEnemy;
    std::function<bool(glm::vec3*)> isTargettedEnemy;

public:
    MilitaryStateMachine();
    glm::vec3 GetTarget() const;
    std::string GetStateDescription() const;
    void Update(float elapsedTime);
};
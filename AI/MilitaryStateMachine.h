#pragma once
#include <functional>
#include <glm\vec3.hpp>

class MilitaryStateMachine
{
public:
    enum State
    {
        FireAtNearest = 0,
        FireAtTargetted = 1,
        TakeCover = 2,
        Flee = 3,
        ThrowExplosive = 4,
        // TravelToFlank, // Future work.
        // TravelToProtect,
        Follow = 7,
        Survey = 8,
        CallForBackup = 9
    };

private:
    std::string parentNpcName;
    State state;
    float timeInState;

    // States of actions.
    std::function<bool()> hasMovedToTarget; 
    std::function<bool()> hasReachedAlly;
    std::function<bool()> hasThrownExplosive;
    std::function<bool()> hasCalledForBackup;

    // States of self and enemies.
    std::function<bool()> isNearbyEnemy;
    std::function<bool()> isTargettedEnemy;
    std::function<bool()> isTargettedAlly;
    std::function<bool()> isEnemyStillAlive;
    std::function<bool()> isManyNearbyEnemies;
    std::function<bool()> isLowPerceivedHealth;

    bool hasNewState;
    void UpdateState(State newState);

public:
    MilitaryStateMachine(const std::string parentNpcName);

    void SetCallbackFunctions(std::function<bool()> hasMovedToTarget, std::function<bool()> hasReachedAlly,
        std::function<bool()> hasThrownExplosive, std::function<bool()> hasCalledForBackup,
        std::function<bool()> isNearbyEnemy, std::function<bool()> isTargettedEnemy,
        std::function<bool()> isTargettedAlly, std::function<bool()> isEnemyStillAlive,
        std::function<bool()> isManyNearbyEnemies, std::function<bool()> isLowPerceivedHealth);

    std::string GetStateDescription() const;
    void Update(float elapsedTime);
    
    State GetState();
    bool HasChangedState();
};
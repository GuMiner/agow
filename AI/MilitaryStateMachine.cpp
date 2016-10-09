#include "MilitaryStateMachine.h"

MilitaryStateMachine::MilitaryStateMachine()
    : state(State::Survey), timeInState(0.0f)
{

}

std::string MilitaryStateMachine::GetStateDescription() const
{
    switch (state)
    {
    case State::CallForBackup:
        return "calling for backup.";
    case State::FireAtNearest:
        return "firing at the enemy.";
    case State::FireAtTargetted:
        return "firing at the target.";
    case State::Flee:
        return "fleeing in panic.";
    case State::Follow:
        return "following the target.";
    case State::Survey:
        return "performing surveillance.";
    case State::TakeCover:
        return "taking cover.";
    case State::ThrowExplosive:
        return "throwing an explosive.";
    case State::TravelToFlank:
        return "traveling to flank the enemy.";
    case State::TravelToProtect:
    default:
        return "traveling to protect the target.";
    }
}

glm::vec3 MilitaryStateMachine::GetTarget() const
{

}

void MilitaryStateMachine::Update(float elapsedTime)
{
    timeInState += elapsedTime;

    // Perform state transitions, using the callback functions for assistance.
    switch (state)
    {
    case State::CallForBackup:
        // TODO
        break;
    case State::FireAtNearest: 
        // TODO
        break;
    case State::FireAtTargetted:
        // TODO
        break;
    case State::Flee:
        // TODO
        break;
    case State::Follow:
        // TODO
        break;
    case State::Survey:
        // If we detect an enemy, fire at it.
        if (isNearbyEnemy(&target))
        {
            state = State::FireAtNearest;
        }

        if (isTargettedEnemy(&target))
        {
            state = State::FireAtTargetted;
        }
        // TODO

        break;
    case State::TakeCover:
        if (hasMovedToTarget())
        {
            state = State::Survey;
        }

        break;
    case State::ThrowExplosive:
        if (hasThrownExplosive())
        {
            state = State::Survey;
        }

        break;
    case State::TravelToFlank:
        // TODO
        break;
    case State::TravelToProtect:
    default:
        // TODO
        break;
    }

}
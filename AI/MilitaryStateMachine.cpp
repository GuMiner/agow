#include "MilitaryStateMachine.h"

MilitaryStateMachine::MilitaryStateMachine()
    : currentState(State::Survey), timeInState(0.0f)
{

}

std::string MilitaryStateMachine::GetStateDescription() const
{
    switch (currentState)
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

void MilitaryStateMachine::Update(float elapsedTime)
{
    timeInState += elapsedTime;

    // Perform state transitions.
    switch (currentState)
    {

    }

}
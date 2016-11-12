#include "logging\Logger.h"
#include "MilitaryStateMachine.h"

MilitaryStateMachine::MilitaryStateMachine(const std::string parentNpcName)
    : parentNpcName(parentNpcName), state(State::Survey), timeInState(0.0f)
{
}

void MilitaryStateMachine::SetCallbackFunctions(std::function<bool()> hasMovedToTarget, std::function<bool()> hasReachedAlly,
    std::function<bool()> hasThrownExplosive, std::function<bool()> hasCalledForBackup,
    std::function<bool()> isNearbyEnemy, std::function<bool()> isTargettedEnemy,
    std::function<bool()> isTargettedAlly, std::function<bool()> isEnemyStillAlive,
    std::function<bool()> isManyNearbyEnemies, std::function<bool()> isLowPerceivedHealth)
{
    this->hasMovedToTarget = hasMovedToTarget;
    this->hasReachedAlly = hasReachedAlly;
    this->hasThrownExplosive = hasThrownExplosive;
    this->hasCalledForBackup = hasCalledForBackup;
    this->isNearbyEnemy = isNearbyEnemy;
    this->isTargettedEnemy = isTargettedEnemy;
    this->isTargettedAlly = isTargettedAlly;
    this->isEnemyStillAlive = isEnemyStillAlive;
    this->isManyNearbyEnemies = isManyNearbyEnemies;
    this->isLowPerceivedHealth = isLowPerceivedHealth;
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
    default:
        return "throwing an explosive.";
    // case State::TravelToFlank:
    //     return "traveling to flank the enemy.";
    // case State::TravelToProtect:
    // default:
    //     return "traveling to protect the target.";
    }
}

void MilitaryStateMachine::Update(float elapsedTime)
{
    timeInState += elapsedTime;
    bool isEnemyAlive;

    // Perform state transitions, using the callback functions for assistance.
    switch (state)
    {
    case State::CallForBackup:
        if (hasCalledForBackup())
        {
            // Once we've called for backup, switch back to survey state.
            UpdateState(State::Survey);
            break;
        }

        // Else continue calling for backup.
        break;
    case State::FireAtNearest: 
        isEnemyAlive = isEnemyStillAlive();
        if (isEnemyAlive && elapsedTime > 5.0f)
        {
            // Take cover if we haven't killed the enemy quickly.
            UpdateState(State::TakeCover);
            break;
        }

        if (!isEnemyAlive)
        {
            // Survey if we killed all nearby enemies.
            UpdateState(State::Survey);
            break;
        }

        // If we had a targetted ally and we are no longer next to it, stop firing and move towards the ally.
        if (isTargettedAlly() && !hasReachedAlly())
        {
            UpdateState(State::Follow);
            break;
        }

        // Stop shooting if there is nobody to shoot.
        if (!isNearbyEnemy())
        {
            UpdateState(State::Survey);
            break;
        }

        // Keep on firing otherwise.
        break;
    case State::FireAtTargetted:
        isEnemyAlive = isEnemyStillAlive();
        if (isEnemyAlive && elapsedTime > 5.0f)
        {
            // Take cover if we haven't killed the enemy recently.
            UpdateState(State::TakeCover);
            break;
        }
        else if (!isEnemyAlive)
        {
            // Survey once the enemy is dead.
            UpdateState(State::Survey);
            break;
        }

        // Stop shooting if there is nobody to shoot.
        if (!isTargettedEnemy())
        {
            UpdateState(State::Survey);
            break;
        }

        // Keep on firing otherwise.
        break;
    case State::Flee:
        if (!isNearbyEnemy())
        {
            // Once we are far away from enemies, stop fleeing.
            UpdateState(State::Survey);
            break;
        }

        // Keep on fleeing otherwise.
        break;
    case State::Follow:
        if (hasReachedAlly())
        {
            // We reached the ally, switch back to survey.
            UpdateState(State::Survey);
            break;
        }

        // Keep on following otherwise.
        break;
    case State::Survey:
        // If we have a targetted ally and aren't at the target, follow it.
        if (isTargettedAlly() && !hasReachedAlly())
        {
            UpdateState(State::Follow);
            break;
        }
        
        // If we detect an enemy, fire at it.
        if (isNearbyEnemy())
        {
            UpdateState(State::FireAtNearest);
            break;
        }

        // If we have a targetted enemy, fire at it.
        if (isTargettedEnemy())
        {
            UpdateState(State::FireAtTargetted);
            break;
        }
        
        if (isManyNearbyEnemies() && isLowPerceivedHealth() && !hasCalledForBackup())
        {
            // Flee if we 'think' we are about to die and there are many nearby enemies
            UpdateState(State::Flee);
            break;
        }

        // Otherwise, keep surveying.
        break;
    case State::TakeCover:
        if (hasMovedToTarget())
        {
            if (isManyNearbyEnemies && isLowPerceivedHealth())
            {
                // Call for backup, which prevents fleeing.
                UpdateState(State::CallForBackup);
                break;
            }

            // Once we are back in cover, we reset to survey state.
            UpdateState(State::Survey);
            break;
        }

        // Otherwise, keep moving to take cover.
        break;
    case State::ThrowExplosive:
    default:
        if (hasThrownExplosive())
        {
            // Once we have thrown the explosive, we reset to survey state.
            UpdateState(State::Survey);
            break;
        }

        // Otherwise, keep throwing the explosive.
        break;
    }
}

MilitaryStateMachine::State MilitaryStateMachine::GetState()
{
    return state;
}

void MilitaryStateMachine::UpdateState(State newState)
{
    Logger::Log(parentNpcName, " switching to state ", newState, " from state ", state);
    hasNewState = true;
    state = newState;
}

bool MilitaryStateMachine::HasChangedState()
{
    bool changedState = hasNewState;
    hasNewState = false;
    return changedState;
}
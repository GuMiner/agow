#include "Events.h"

Events::Events()
    : events(), id(0)
{
}

int Events::AddPeriodicEvent(TypedCallback<EventType> callback, float repeatTimeInSec, bool startImmediately)
{
    ++id;
    events.push_front(InternalEvent(id, InternalEventType::PERIODIC, callback, repeatTimeInSec, 0));
    if (startImmediately)
    {
        callback.CallCallback();
    }

    return id;
}

int Events::AddSingleEvent(TypedCallback<EventType> callback, float startAfterTimeInSec)
{
    ++id;
    if (startAfterTimeInSec <= 0)
    {
        callback.CallCallback();
    }
    else
    {
        events.push_front(InternalEvent(id, InternalEventType::SINGLE_OFFSET_TIME, callback, startAfterTimeInSec, 0));
    }

    return id;
}

int Events::AddGameTimeEvent(TypedCallback<EventType> callback, float gameTimeInSec)
{
    ++id;
    events.push_front(InternalEvent(id, InternalEventType::SINGLE_GAME_TIME, callback, gameTimeInSec, 0));
    return id;
}

void Events::UpdateEvents(float gameTime, float elapsedTime)
{
    for (std::list<InternalEvent>::iterator iter = events.begin(); iter != events.end();)
    {
        bool erased = false;
        switch (iter->internalType)
        {
        case InternalEventType::PERIODIC:
            iter->elapsedTime += elapsedTime;
            if (iter->elapsedTime > iter->triggerTime)
            {
                iter->callback.CallCallback();
                iter->elapsedTime -= iter->triggerTime;
            }

            break;
        case InternalEventType::SINGLE_GAME_TIME:
            if (gameTime > iter->triggerTime)
            {
                iter->callback.CallCallback();
                iter = events.erase(iter);
                erased = true;
            }

            break;
        case InternalEventType::SINGLE_OFFSET_TIME:
            iter->elapsedTime += elapsedTime;
            if (iter->elapsedTime > iter->triggerTime)
            {
                iter->callback.CallCallback();
                iter = events.erase(iter);
                erased = true;
            }

            break;
        default:
            break;
        }

        if (!erased)
        {
            iter++;
        }
    }
}

void Events::Cancel(int id)
{
    for (std::list<InternalEvent>::iterator iter = events.begin(); iter != events.end(); iter++)
    {
        if (iter->id == id)
        {
            events.erase(iter);
            return;
        }
    }
}

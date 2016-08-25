#include "Events.h"



Events::Events()
    : events()
{
}

void Events::AddPeriodicEvent(TypedCallback<EventType> callback, float repeatTimeInSec, bool startImmediately)
{
}

void Events::AddSingleEvent(TypedCallback<EventType> callback, float startAfterTimeInSec)
{

}

void Events::AddGameTimeEvent(TypedCallback<EventType> callback, float gameTimeInSec)
{

}

void Events::UpdateEvents(float gameTime, float elapsedTime)
{

}

#pragma once
#include <list>
#include "Utils\TypedCallback.h"

enum EventType
{
    SKY_FLAVOR_COLOR_CHANGE
};

enum InternalEventType
{
    PERIODIC,
    SINGLE_OFFSET_TIME,
    SINGLE_GAME_TIME
};

struct Event
{
    InternalEventType internalType;
    TypedCallback<EventType> callback;

    float triggerTime;
    float elapsedTime;
};

class Events
{
    std::list<Event> events;

public:
    Events();

    void AddPeriodicEvent(TypedCallback<EventType> callback, float repeatTimeInSec, bool startImmediately);
    void AddSingleEvent(TypedCallback<EventType> callback, float startAfterTimeInSec);
    void AddGameTimeEvent(TypedCallback<EventType> callback, float gameTimeInSec);

    void UpdateEvents(float gameTime, float elapsedTime);
};


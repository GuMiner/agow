#pragma once
#include <list>
#include "Data\CallbackTypes.h"
#include "Utils\TypedCallback.h"

enum InternalEventType
{
    PERIODIC,
    SINGLE_OFFSET_TIME,
    SINGLE_GAME_TIME
};

struct InternalEvent
{
    int id;

    InternalEventType internalType;
    TypedCallback<EventType> callback;

    float triggerTime;
    float elapsedTime;

    InternalEvent(int id, InternalEventType internalType, TypedCallback<EventType> callback, float triggerTime, float elapsedTime)
        : id(id), internalType(internalType), callback(callback), triggerTime(triggerTime), elapsedTime(elapsedTime)
    {}
};

// Manages a list of events that are updateable, can run, and are cancelleable.
class Events
{
    std::list<InternalEvent> events;
    int id;

public:
    Events();

    int AddPeriodicEvent(TypedCallback<EventType> callback, float repeatTimeInSec, bool startImmediately);
    int AddSingleEvent(TypedCallback<EventType> callback, float startAfterTimeInSec);
    int AddGameTimeEvent(TypedCallback<EventType> callback, float gameTimeInSec);

    void UpdateEvents(float gameTime, float elapsedTime);

    void Cancel(int id);
};


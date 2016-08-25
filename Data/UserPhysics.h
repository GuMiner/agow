#pragma once
#include "Math\Vec.h"

// Copied from PaletteWindow.cpp, MapEditor.
enum ObjectType
{
	HEIGHTMAP = 0,
	PLAYER = 1,
    NPC_CLOSEUP = 2
};

class ICollisionCallback
{
public:
    virtual ~ICollisionCallback()
    {
    }

    virtual void CollisionCallback(ObjectType collidingObject) = 0;
};

struct UserPhysics
{
	ObjectType objectType;
    ICollisionCallback* callback;
	
    UserPhysics(ObjectType objectType, ICollisionCallback* callback = nullptr)
        : objectType(objectType), callback(callback)
    {
    }

    void CallCallback(ObjectType collidingObject)
    {
        if (callback != nullptr)
        {
            callback->CollisionCallback(collidingObject);
        }
    }

    // Returns true if source collides with target. If so, target's collision callback should be called.
    static bool Collides(ObjectType target, ObjectType source)
    {
        switch (target)
        {
        case PLAYER:
            switch(source)
            {
            case HEIGHTMAP:
                return true;
            default:
                return false;
            }
        case NPC_CLOSEUP:
            switch (source)
            {
            case PLAYER:
                return true;
            default:
                return false;
            }
        default:
            return false;
        }
    }
};
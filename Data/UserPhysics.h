#pragma once
#include <glm\vec3.hpp>

class UserPhysics
{
public:
    // The type of object this is
    enum ObjectType
    {
        HEIGHTMAP = 0,
        PLAYER = 1,
        NPC_CLOSEUP = 2,
        BUILDING_COVER = 3,
        BUILDING_SEGMENT = 4,
        ROCK = 5,
        PLASMA_BALL = 6
    };

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
        case BUILDING_COVER:
            return true;
        case BUILDING_SEGMENT:
            return false; // Segments have a type, but don't have callbacks.
        case ROCK:
            return false; // TODO implement rock/NPC collisions.
        case PLASMA_BALL:
            return false; // TODO implement plasma ball / NPC collisions.
        default:
            return false;
        }
    }
};
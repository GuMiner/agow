#pragma once
#include "Math\Vec.h"

class UserPhysics
{
public:
    // Copied from PaletteWindow.cpp, MapEditor.
    enum ObjectType
    {
        HEIGHTMAP = 0,
        PLAYER = 1,
        NPC_CLOSEUP = 2
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
        default:
            return false;
        }
    }
};
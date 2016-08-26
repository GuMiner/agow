#pragma once
#include <map>
#include "Math\Vec.h"

enum EventType
{
    SKY_FLAVOR_COLOR_CHANGE,
    SKY_FLAVOR_RANDOM_COLOR_CHANGE
};

struct SkyColorChange
{
    vec::vec3 flavorColor;
    float strength;
};
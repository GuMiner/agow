#pragma once
#include <string>
#include "Math\Vec.h"

struct StyleText
{
    enum Effect
    {
        NORMAL,
        ITALICS,
        MINI,
        UNDERLINED
    };

    std::string text;
    vec::vec3 color;
    Effect effect;
};
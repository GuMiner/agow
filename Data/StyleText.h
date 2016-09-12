#pragma once
#include <string>
#include "Math\Vec.h"

struct StyleText
{
    enum Effect
    {
        NORMAL,
        ITALICS,
        BOLD,
        UNDERLINED
    };

    std::string text;
    vec::vec3 color;
    Effect effect;
};
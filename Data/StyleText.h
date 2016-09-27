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

    StyleText()
    {
    }

    // Creates bland-system-level conversational text.
    StyleText(std::string text, Effect effect)
        : text(text), color(vec::vec3(0.0f, 1.0f, 0.0f)), effect(effect)
    {
    }
};
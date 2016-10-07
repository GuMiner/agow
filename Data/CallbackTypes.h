#pragma once
#include <map>
#include <glm\vec3.hpp>
#include "Data\StyleText.h"

enum EventType
{
    SKY_FLAVOR_COLOR_CHANGE,
    SKY_FLAVOR_RANDOM_COLOR_CHANGE,
    ADD_DIALOG
};

struct SkyColorChange
{
    glm::vec3 flavorColor;
    float strength;
};

struct DialogData
{
    StyleText dialogText;

    DialogData()
    {
    }

    DialogData(StyleText text)
        : dialogText(text)
    {
    }
};
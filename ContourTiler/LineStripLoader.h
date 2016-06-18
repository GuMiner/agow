#pragma once
#include <string>
#include "Definitions.h"

class LineStripLoader
{
public:
    static bool LoadLineStrips(std::string fileName, std::vector<LineStrip>& lineStrips);
};


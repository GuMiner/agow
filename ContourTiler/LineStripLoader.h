#pragma once
#include <string>
#include "Definitions.h"

class LineStripLoader
{
public:
    // Given a filename, fills in the vector with all the line strips. Returns true on success, false otherwise.
    static bool LoadLineStrips(std::string fileName, std::vector<LineStrip>* lineStrips);
};


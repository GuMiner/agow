#pragma once
#include <vector>

namespace Constant
{
    const std::string ContourFile = "smalldata.bin";
}

// Data format we read in (~1 wk running on 1 thread to compute).
struct Point
{
    unsigned short x, y;
};

struct LineStrip
{
    unsigned short elevation;
    std::vector<Point> points;
};
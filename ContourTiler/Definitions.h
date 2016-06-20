#pragma once
#include <vector>

namespace Constant
{
    const std::string ContourFile = "contours.bin";
}

// Data format we read in (~1 wk running on 1 thread to compute).
struct Point
{
    double x, y;
};

struct LineStrip
{
    double elevation;
    std::vector<Point> points;
};
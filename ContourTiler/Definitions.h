#pragma once
#include <vector>

struct Point
{
    double x, y;
};

struct LineStrip
{
    double elevation;
    std::vector<Point> points;
};
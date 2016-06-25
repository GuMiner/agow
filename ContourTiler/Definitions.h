#pragma once
#include <vector>

namespace Constant
{
    const std::string ContourFile = "data.bin";
    const std::string ContourIndexFile = "indices.bin";
    const std::string ElevationIndexFile = "elevations.bin";
    
    const double XMin = 1214976.7500000000;
    const double XMax = 1583489.0000000000;
    
    const double YMin = 22489.217250004411;
    const double YMax = 360015.64550000429;
    const double ZMin = -900.00000000000000;
    const double ZMax = 7960.0000000000000;
}

// Lines are construed to be from the current point to the next point.
struct Index
{
    int stripIdx;
    int pointIdx;
};

// Point data format.
struct Point
{
    double x, y;

    Point()
    { }

    Point(double xx, double yy) : x(xx), y(yy)
    { }
};

// Line data format.
struct LineStrip
{
    double elevation;
    std::vector<Point> points;
};
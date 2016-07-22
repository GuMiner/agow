#pragma once
#include <string>
#include <vector>
#include <map>

struct Point
{
    double x;
    double y;
};

// Points that logically form a line.
struct LineStrip
{
    std::vector<Point> points;
};

class RoadFeatures
{
public:
    enum RoadSizes
    {
        AVE = 0,
        ST = 1,
        PL = 2,
        WAY = 3,
        FWY = 4
    };

    std::map<RoadSizes, std::vector<LineStrip>> roads;
    std::vector<Point> barricades;
    std::vector<Point> stops;
    std::vector<Point> emitters;
    std::vector<Point> freewayModifiers;

private:
    bool LoadPointFeatures(std::string filename, std::vector<Point>& pointStorage);

public:
    RoadFeatures();

    bool LoadRoadFeatures();
};


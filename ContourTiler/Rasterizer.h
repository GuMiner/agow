#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <SFML\System.hpp>
#include "Definitions.h"

// Lines are construed to be from the current point to the next point.
struct Index
{
    int stripIdx;
    int pointIdx;
};

class Rasterizer
{
    double xMax, yMax;

    // The number of quadtree xy grid spaces.
    int divisionFactor;
    std::vector<std::vector<Index>> quadtree;
    
    // True if the line intersects th given quad, false otherwise.
    bool DoesLineIntersect(sf::Vector2<double> start, sf::Vector2<double> end, sf::Rect<double> quadArea);

    // Gets the point position given the iterator (reading at the iterator's current position).
    Point GetPoint(std::vector<LineStrip>& lineStrips, std::vector<Index>::iterator iter);

    // Given an index, finds the distance from this point to it.
    double GetDistance(std::vector<LineStrip>& lineStrips, Index idx, double x, double y);

    // Same as the above but treats the index as a line.
    double GetLineDistance(std::vector<LineStrip>& lineStrips, Index idx, double x, double y);

    // Adds an area if it is valid.
    void AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Adds areas to search given the current point and distance away from it.
    void AddAreasToSearch(int distance, int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Returns the height of the closest point to the specified coordinates.
    double FindClosestPoint(std::vector<LineStrip>& lineStrips, double x, double y);

public:
    Rasterizer(double xMax, double yMax, int divisionFactor);

    // Setup to be done before rasterization can be performed.
    void Setup(std::vector<LineStrip>& lineStrips);

    // Rasterizes the area, filling in the raster store.
    void Rasterize(std::vector<LineStrip>& lineStrips, sf::Rect<double> boundingBox, int width, int height, double** rasterStore, double& minElevation, double& maxElevation);

    virtual ~Rasterizer();
};


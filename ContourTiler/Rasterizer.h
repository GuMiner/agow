#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include "Definitions.h"

struct Index
{
    int stripIdx;
    int pointIdx;
};

class Rasterizer
{
    double xMax, yMax;
    std::vector<LineStrip>* lineStrips;

    // The number of quadtree xy grid spaces.
    int divisionFactor;
    std::vector<std::vector<Index>> quadtree;
    
    // Gets the point position given the iterator (reading at the iterator's current position).
    Point GetPoint(std::vector<Index>::iterator iter);

    // Given an index, finds the elevation.
    double GetDistance(Index idx, double x, double y);

    // Adds an area if it is valid.
    void AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Adds areas to search given the current point and distance away from it.
    void AddAreasToSearch(int distance, int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Returns the height of the closest point to the specified coordinates.
    double FindClosestPoint(double x, double y);

public:
    Rasterizer(double xMax, double yMax, int divisionFactor);

    // Setup to be done before rasterization can be performed.
    void Setup(std::vector<LineStrip>* lineStrips);

    // Rasterizes the area, filling in the raster store.
    void Rasterize(sf::Rect<double> boundingBox, int width, int height, double** rasterStore, double& minElevation, double& maxElevation);

    virtual ~Rasterizer();
};


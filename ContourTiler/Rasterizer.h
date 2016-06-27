#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include "Definitions.h"
#include "LineStripLoader.h"
#include "Quadtree.h"

class Rasterizer
{
    LineStripLoader* lineStrips;
    Quadtree quadtree;

    // The number of quadtree xy grid spaces.
    int size;

    sf::Vector2i GetQuadtreeSquare(Point givenPoint);

    void AddLinePortionsToQuadtree(sf::Vector2i quadStart, Index index, Point startPoint, Point endPoint);

    // Same as the above but treats the index as a line.
    decimal GetPointToPointAngle(Point start, Point end);
    decimal GetLineDistanceWithAngle(Index idx, Point point, decimal* angle);

    // Adds an area if it is valid.
    void AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Adds areas to search given the current point and distance away from it.
    void AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads);

    // Returns the height of the closest point to the specified coordinates.
    int GetQuadrant(decimal angle);
    decimal FindClosestPoint(Point point);

    // Rasterizes a range of columns to improve perf.
    void RasterizeColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore, double* minElevation, double* maxElevation);

public:
    Rasterizer(LineStripLoader* lineStripLoader, int size);

    // Setup to be done before rasterization can be performed.
    void Setup();

    // Rasterizes the area, filling in the raster store.
    void Rasterize(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore, double& minElevation, double& maxElevation);

    // Rasterizes in lines with full whiteness.
    void LineRaster(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore);

    virtual ~Rasterizer();
};


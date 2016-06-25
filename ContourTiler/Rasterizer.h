#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include "Definitions.h"
#include "LineStripLoader.h"

class Rasterizer
{
    LineStripLoader* lineStrips;

    // The number of quadtree xy grid spaces.
    int size;
    std::vector<std::vector<Index>> quadtree;
    
    sf::Vector2i GetQuadtreeSquare(Point givenPoint);

    // True if the line intersects th given quad, false otherwise.
    bool DoesLineIntersect(sf::Vector2<double> start, sf::Vector2<double> end, sf::Rect<double> quadArea);

    // Same as the above but treats the index as a line.
    double GetLineDistance(Index idx, Point point);

    // Adds an area if it is valid.
    void AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Adds areas to search given the current point and distance away from it.
    void AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads);

    // Returns the height of the closest point to the specified coordinates.
    double FindClosestPoint(Point point);

public:
    Rasterizer(LineStripLoader* lineStripLoader, int size);

    // Setup to be done before rasterization can be performed.
    void Setup();

    // Rasterizes the area, filling in the raster store.
    void Rasterize(sf::Rect<double> boundingBox, double** rasterStore, double& minElevation, double& maxElevation);

    // Rasterizes in lines with full whiteness.
    void LineRaster(sf::Rect<double> boundingBox, double** rasterStore);

    virtual ~Rasterizer();
};


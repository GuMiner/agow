#include <cmath>
#include <thread>
#include <iostream>
#include <limits>
#include <map>
#include "Rasterizer.h"

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader, int size)
    : lineStrips(lineStripLoader), quadtree(size), size(size)
{
}

sf::Vector2i Rasterizer::GetQuadtreeSquare(Point givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * size), size - 1),
        std::min((int)(givenPoint.y * size), size - 1));
}

void Rasterizer::Setup()
{
    quadtree.InitializeQuadtree();

    // Now fill in all the quadtree files with the indexes of all the lines within the area.
    long pointCount = 0;
    for (int i = 0; i < lineStrips->lineStrips.size(); i++)
    {
        std::vector<Point>& points = lineStrips->lineStrips[i].points;

        for (unsigned int j = 0; j < points.size() - 1; j++)
        {
            sf::Vector2i quadStart = GetQuadtreeSquare(points[j]);
            sf::Vector2i quadEnd = GetQuadtreeSquare(points[j + 1]);
            Index index(i, j);

            quadtree.AddToIndex(quadStart, index);

            if (quadEnd.x != quadStart.x || quadEnd.y != quadStart.y)
            {
                quadtree.AddToIndex(quadEnd, index);
            }

            ++pointCount;
            if (pointCount % 1000000 == 0)
            {
                std::cout << "Quadtree processed point " << pointCount << ". (" << (float)pointCount / 1.5e7f << "%)" << std::endl;
            }
        }
    }

    std::cout << "Quadtree initialized!" << std::endl;
}

// Same as the above but treats the index as a line.
decimal Rasterizer::GetLineDistance(Index idx, Point point)
{
    Point start = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx];
    Point end = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1];
    
    decimal startEndLength = sqrt(pow(end.x - start.x, 2) + pow(end.y - start.y, 2));
    Point startToEnd(end.x - start.x, end.y - start.y);
    Point startToPoint(point.x - start.x, point.y - start.y);
    decimal projectionLength = (startToPoint.x * startToEnd.x + startToPoint.y * startToEnd.y) / startEndLength;

    if (projectionLength > 0 && projectionLength < startEndLength)
    {
        decimal fraction = projectionLength / startEndLength;
        Point closestPoint(start.x + startToEnd.x * fraction, start.y + startToEnd.y * fraction);

        return sqrt(pow(closestPoint.x - point.x, 2) + pow(closestPoint.y - point.y, 2));
    }
    else if (projectionLength < 0)
    {
        return sqrt(pow(start.x - point.x, 2) + pow(start.y - point.y, 2));
    }

    return sqrt(pow(end.x - point.x, 2) + pow(end.y - point.y, 2));
}

void Rasterizer::AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    if (xP >= 0 && yP >= 0 && xP < size && yP < size && quadtree.QuadSize(sf::Vector2i(xP, yP)) != 0)
    {
        searchQuads.push_back(sf::Vector2i(xP, yP));
    }
}

void Rasterizer::AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads)
{
    if (distance == 0)
    {
        searchQuads.push_back(startQuad);
    }
    else
    {
        // Add the horizontal bars.
        for (int i = startQuad.x - distance; i <= startQuad.x + distance; i++)
        {
            AddIfValid(i, startQuad.y + distance, searchQuads);
            AddIfValid(i, startQuad.y - distance, searchQuads);
        }

        // Add the vertical bars, skipping the corners that otherwise would be duplicated.
        for (int j = startQuad.y - (distance - 1); j <= startQuad.y + (distance - 1); j++)
        {
            AddIfValid(startQuad.x + distance, j, searchQuads);
            AddIfValid(startQuad.x - distance, j, searchQuads);
        }
    }
}

decimal Rasterizer::FindClosestPoint(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);

    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 0;
    std::vector<sf::Vector2i> searchQuads;

    bool foundAPoint = false;
    int overrun = 0;
    const int overrunLimit = 7; // Empirically determined to be 'ok'
    std::map<int, decimal> closestElevations;
    std::map<int, decimal> closestDistances;
    while (true)
    {
        searchQuads.clear();
        AddAreasToSearch(gridDistance, quadSquare, searchQuads);

        for (int k = 0; k < searchQuads.size(); k++)
        {
            int indexCount = quadtree.QuadSize(searchQuads[k]);
            for (size_t i = 0; i < indexCount; i++)
            {
                Index index = quadtree.GetIndexFromQuad(searchQuads[k], i);
                
                decimal lineDist = GetLineDistance(index, point);
                if (lineDist < 1e-12)
                {
                    return lineStrips->lineStrips[index.stripIdx].elevation;
                }

                // int quadrant = GetQuadrant(angle);

                // See if there's a closest line for this elevation and replace as needed
                int elevationId = lineStrips->lineStrips[index.stripIdx].elevationId;
                if (closestDistances.find(elevationId) != closestDistances.end())
                {
                    if (lineDist < closestDistances[elevationId])
                    {
                        closestDistances[elevationId] = lineDist;
                    }
                }
                else
                {
                    closestElevations[elevationId] = lineStrips->lineStrips[index.stripIdx].elevation;
                    closestDistances[elevationId] = lineDist;
                }

                foundAPoint = true;
            }
        }

        if (foundAPoint && (overrun >= overrunLimit || closestElevations.size() > 1))
        {
            decimal elevation = 0;
            decimal inverseWeights = 0;

            for (std::map<int, decimal>::iterator iter = closestElevations.begin(); iter != closestElevations.end(); iter++)
            {
                int elevationId = iter->first;
                elevation += iter->second / closestDistances[elevationId];
                inverseWeights += 1.0 / closestDistances[elevationId];
            }

            return elevation / inverseWeights;
        }
        else if (foundAPoint)
        {
            // Overrun a few times to account for edge quadtree conditions.
            ++overrun;
        }

        // Increment the grids we search.
        ++gridDistance;
    }
}

// Rasterizes a range of columns to improve perf.
void Rasterizer::RasterizeColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore, double* minElevation, double* maxElevation)
{
    for (int i = startColumn; i < startColumn + columnCount; i++)
    {
        for (int j = 0; j < size; j++)
        {
            decimal x = leftOffset + ((decimal)i / (decimal)size) * effectiveSize;
            decimal y = topOffset + ((decimal)j / (decimal)size) * effectiveSize;

            Point point(x, y);
            decimal elevation = FindClosestPoint(point);
            (*rasterStore)[i + j * size] = elevation;

            if (elevation < *minElevation)
            {
                *minElevation = elevation;
            }

            if (elevation > *maxElevation)
            {
                *maxElevation = elevation;
            }
        }

        if (i % 20 == 0)
        {
            std::cout << "Rasterized line " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Thread from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
}

void Rasterizer::Rasterize(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore, double& minElevation, double& maxElevation)
{
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();

    const int splitFactor = 7;
    double minElevations[splitFactor];
    double maxElevations[splitFactor];
    std::thread* threads[splitFactor];

    int range = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
        minElevations[i] = std::numeric_limits<double>::max();
        maxElevations[i] = std::numeric_limits<double>::min();
        threads[i] = new std::thread(&Rasterizer::RasterizeColumnRange, this, leftOffset, topOffset, effectiveSize, i * range, range, rasterStore, &minElevations[i], &maxElevations[i]);
    }

    std::cout << "Rasterizing..." << std::endl;
    
    for (int i = 0; i < splitFactor; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    // Find the real min and max.
    for (int i = 0; i < splitFactor; i++)
    {
        if (minElevations[i] < minElevation)
        {
            minElevation = minElevations[i];
        }

        if (maxElevations[i] > maxElevation)
        {
            maxElevation = maxElevations[i];
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

void Rasterizer::LineRaster(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore)
{
    std::cout << "Line Rasterizing..." << std::endl;
    int m = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            decimal x = leftOffset + ((decimal)i / (decimal)size) * effectiveSize;
            decimal y = topOffset + ((decimal)j / (decimal)size) * effectiveSize;
            decimal wiggleDist = effectiveSize / (decimal)size;

            Point point(x, y);

            sf::Vector2i quadSquare = GetQuadtreeSquare(point);
           
            for (size_t k = 0; k < quadtree.QuadSize(quadSquare); k++)
            {
                Index index = quadtree.GetIndexFromQuad(quadSquare, k);
                
                /* 
                // Display the points individually.
                Point newPoint = lineStrips->lineStrips[index.stripIdx].points[index.pointIdx];
                decimal ii = (newPoint.x - leftOffset) / effectiveSize;
                decimal jj = (newPoint.y - topOffset) / effectiveSize;

                if (ii > 0 && jj > 0 && ii < 1 && jj < 1)
                {
                    (*rasterStore)[(int)(ii * size) + (int)(jj * size) * size] = 1e6;
                }*/

                decimal lineDist = GetLineDistance(index, point);
                if (lineDist < wiggleDist)
                {
                    (*rasterStore)[i + j * size] = 2e8; // Bigger than anything we'll hit.
                    break;
                }
            }
        }

        if (i % 20 == 0)
        {
            std::cout << "Line rasterized line " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

Rasterizer::~Rasterizer()
{
}

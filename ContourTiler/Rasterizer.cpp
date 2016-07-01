#include <cmath>
#include <thread>
#include <iostream>
#include <limits>
#include <map>
#include "CloseContourRanker.h"
#include "Rasterizer.h"

double toggle;

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader, QuadExclusions* quadExclusions, int size)
    : lineStrips(lineStripLoader), quadExclusions(quadExclusions), quadtree(size), size(size)
{
    toggle = 0;
}


sf::Vector2i Rasterizer::GetQuadtreeSquare(Point givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * (decimal)size + (decimal)toggle), size - 1),
        std::min((int)(givenPoint.y * (decimal)size + (decimal)toggle), size - 1));
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

    toggle = 0;

    std::cout << "Quadtree initialized!" << std::endl;
}

// Same as the above but treats the index as a line.
decimal Rasterizer::GetLineDistanceSqd(Index idx, Point point)
{
    Point& start = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx];
    Point& end = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1];
    
    Point startToEnd(end.x - start.x, end.y - start.y);
    decimal startEndLengthSqd = pow(startToEnd.x, 2) + pow(startToEnd.y, 2);
    
    // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
    Point startToPoint(point.x - start.x, point.y - start.y);
    decimal projectionFraction = (startToPoint.x * startToEnd.x + startToPoint.y * startToEnd.y) / startEndLengthSqd;

    if (projectionFraction > 0 && projectionFraction < 1)
    {
        Point closestPoint(start.x + startToEnd.x * projectionFraction, start.y + startToEnd.y * projectionFraction);
        return pow(closestPoint.x - point.x, 2) + pow(closestPoint.y - point.y, 2);
    }
    else if (projectionFraction < 0)
    {
        return pow(start.x - point.x, 2) + pow(start.y - point.y, 2);
    }

    return pow(end.x - point.x, 2) + pow(end.y - point.y, 2);
}

void Rasterizer::AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    sf::Vector2i pt(xP, yP);
    if (xP >= 0 && yP >= 0 && xP < size && yP < size && !quadExclusions->IsExcluded(pt) && quadtree.QuadSize(pt) != 0)
    {
        searchQuads.push_back(sf::Vector2i(xP, yP));
    }
}

void Rasterizer::AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads)
{
    if (distance == 1)
    {
        searchQuads.push_back(startQuad);
    }

    // Add the horizontal bars
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

decimal Rasterizer::FindClosestPoint(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);

    if (quadExclusions->IsExcluded(quadSquare))
    {
        // The point the point is within is excluded. Return immediately.
        return 2e8;
    }

    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 1;
    std::vector<sf::Vector2i> searchQuads;

    bool foundAPoint = false;
    int overrun = 0;
    const int overrunLimit = 5; // Empirically determined to be 'ok'
    CloseContourRanker contourRanker = CloseContourRanker();
    while (true)
    {
        searchQuads.clear();
        AddAreasToSearch(gridDistance, quadSquare, searchQuads);

        for (int k = 0; k < searchQuads.size(); k++)
        {
            int indexCount = (int)quadtree.QuadSize(searchQuads[k]);
            for (size_t i = 0; i < indexCount; i++)
            {
                Index index = quadtree.GetIndexFromQuad(searchQuads[k], (int)i);
                
                decimal lineDistSqd = GetLineDistanceSqd(index, point);
                if (lineDistSqd < 1e-12)
                {
                    return (decimal)lineStrips->lineStrips[index.stripIdx].elevation;
                }

                // Add to the ranker, which manages priority of lines.
                int elevationId = lineStrips->lineStrips[index.stripIdx].elevationId;
                contourRanker.AddElevationToRank(CloseContourLine(lineDistSqd, elevationId, (decimal)lineStrips->lineStrips[index.stripIdx].elevation));
                foundAPoint = true;
            }
        }

        if (foundAPoint && (overrun >= overrunLimit || (contourRanker.FilledSufficientLines() && overrun != 0)))
        {
            return contourRanker.GetWeightedElevation();
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

            // Avoid scaling based on the exclusion lists.
            if (elevation < 1e8)
            {
                if (elevation < *minElevation)
                {
                    *minElevation = elevation;
                }

                if (elevation > *maxElevation)
                {
                    *maxElevation = elevation;
                }
            }
        }

        if (i % 50 == 0)
        {
            std::cout << "Rasterized " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Thread from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
}

void Rasterizer::Rasterize(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore, double& minElevation, double& maxElevation)
{
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();

    const int splitFactor = 8;
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

// Rasterizes a range of lines to improve perf.
void Rasterizer::RasterizeLineColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore)
{
    for (int i = startColumn; i < startColumn + columnCount; i++)
    {
        for (int j = 0; j < size; j++)
        {
            decimal x = leftOffset + ((decimal)i / (decimal)size) * effectiveSize;
            decimal y = topOffset + ((decimal)j / (decimal)size) * effectiveSize;
            decimal wiggleDistSqd = pow(effectiveSize / (decimal)size, 2);

            Point point(x, y);
            sf::Vector2i quadSquare = GetQuadtreeSquare(point);

            bool filled = false;
            for (size_t k = 0; k < quadtree.QuadSize(quadSquare); k++)
            {
                Index index = quadtree.GetIndexFromQuad(quadSquare, (int)k);

                decimal lineDistSqd = GetLineDistanceSqd(index, point);
                if (lineDistSqd < wiggleDistSqd)
                {
                    filled = true;
                    break;
                }
            }

            (*rasterStore)[i + j * size] = filled ? 1 : 0;
        }

        if (i % 20 == 0)
        {
            std::cout << "Line rendered " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Thread from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
}

void Rasterizer::LineRaster(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore)
{
    // This could also be parallelized.
    std::cout << "Line Rasterizing..." << std::endl;
    
    const int splitFactor = 8;
    std::thread* threads[splitFactor];

    int range = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
        threads[i] = new std::thread(&Rasterizer::RasterizeLineColumnRange, this, leftOffset, topOffset, effectiveSize, i * range, range, rasterStore);
    }

    std::cout << "Rasterizing..." << std::endl;

    for (int i = 0; i < splitFactor; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    std::cout << "Rasterization complete." << std::endl;
}

Rasterizer::~Rasterizer()
{
}

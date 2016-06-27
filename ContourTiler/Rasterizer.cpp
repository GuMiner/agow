#include <cmath>
#include <thread>
#include <iostream>
#include <limits>
#include "Rasterizer.h"

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader, int size)
    : lineStrips(lineStripLoader), quadtree(size), size(size)
{
}

void Rasterizer :: AddLinePortionsToQuadtree(sf::Vector2i quadStart, Index index, Point startPoint, Point endPoint)
{
    quadtree.AddToIndex(quadStart, index);
    
    const decimal stepSize = (1.0 / (decimal)size) / 3; // This will miss tight corners but will be acceptable for my application.

    const Point startToEnd(endPoint.x - startPoint.x, endPoint.y - startPoint.y);
    const decimal startToEndLength = sqrt(pow(startToEnd.x, 2) + pow(startToEnd.y, 2));
    
    sf::Vector2i currentQuad(quadStart.x, quadStart.y);
    decimal currentLength = stepSize;
    while (currentLength < startToEndLength)
    {
        decimal fraction = currentLength / startToEndLength;
        Point newPoint(startPoint.x + fraction * startToEnd.x, startPoint.y + fraction * startToEnd.y);

        sf::Vector2i newQuad = GetQuadtreeSquare(newPoint);
        if (newQuad.x != currentQuad.x || newQuad.y != currentQuad.y)
        {
            quadtree.AddToIndex(newQuad, index);
            currentQuad = newQuad;
        }

        currentLength += stepSize;
    }

    sf::Vector2i endQuad = GetQuadtreeSquare(endPoint);
    if (endQuad.x != currentQuad.x || endQuad.y != currentQuad.y)
    {
        quadtree.AddToIndex(endQuad, index);
    }

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
    for (int i = 0; i < lineStrips->GetStripCount(); i++)
    {
        std::vector<Point>& points = lineStrips->lineStrips[i].points;
        // lineStrips->ReadAllPoints(i, points);

        for (unsigned int j = 0; j < points.size() - 1; j++)
        {
            sf::Vector2i lineStart = GetQuadtreeSquare(points[j]);
            Index index(i, j);
            AddLinePortionsToQuadtree(lineStart, index, points[j], points[j + 1]);
        }

        if (i % 100 == 0)
        {
            std::cout << "Added strip " << i << " of " << lineStrips->GetStripCount() << " to the quadtree." << std::endl;
        }
    }

    std::cout << "Quadtree initialized!" << std::endl;
}

decimal Rasterizer::GetPointToPointAngle(Point start, Point end)
{
    return std::atan2(end.y - start.y, end.x - start.x);
}

// Same as the above but treats the index as a line.
decimal Rasterizer::GetLineDistanceWithAngle(Index idx, Point point, decimal* angle)
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

        *angle = GetPointToPointAngle(point, closestPoint);
        return sqrt(pow(closestPoint.x - point.x, 2) + pow(closestPoint.y - point.y, 2));
    }
    else if (projectionLength < 0)
    {
        *angle = GetPointToPointAngle(point, start);
        return sqrt(pow(start.x - point.x, 2) + pow(start.y - point.y, 2));
    }

    *angle = GetPointToPointAngle(point, end);
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

int Rasterizer::GetQuadrant(decimal angle)
{
    // Angle is from -pi to pi. Split it into the appropriate bucket.
    const decimal pi = 3.14159;
    int quadrant;
    if (angle > -pi && angle < -3 * pi / 4)
    {
        quadrant = 0;
    }
    else if (angle > -3 * pi / 4 && angle < -pi / 2)
    {
        quadrant = 1;
    }
    else if (angle > -pi / 2 && angle < -pi / 4)
    {
        quadrant = 2;
    }
    else if (angle > -pi / 4 && angle < 0)
    {
        quadrant = 3;
    }
    else if (angle > 0 && angle < pi / 4)
    {
        quadrant = 4;
    }
    else if (angle > pi / 4 && angle < pi / 2)
    {
        quadrant = 5;
    }
    else if (angle > pi / 2 && angle < 3 * pi / 4)
    {
        quadrant = 6;
    }
    else
    {
        quadrant = 7;
    }

    return quadrant;
}

decimal Rasterizer::FindClosestPoint(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);

    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 0;
    std::vector<sf::Vector2i> searchQuads;

    // FYI, this is now angle-restricted inverse distance weighting.
    
    const int quadrants = 8;
    decimal totalElevations[quadrants];
    decimal totalInverseDistances[quadrants];
    int pointTotal[quadrants];
    for (int i = 0; i < quadrants; i++)
    {
        totalElevations[i] = 0;
        totalInverseDistances[i] = 0;
        pointTotal[i] = 0;
    }

    bool foundAPoint = false;
    int overrun = 0;
    const int overrunLimit = 15;
    while (true)
    {
        searchQuads.clear();
        AddAreasToSearch(gridDistance, quadSquare, searchQuads);

        for (int k = 0; k < searchQuads.size(); k++)
        {
            int indexCount = quadtree.QuadSize(searchQuads[k]);
            for (size_t i = 0; i < indexCount; i++)
            {
                decimal angle;
                decimal lineDist = GetLineDistanceWithAngle(quadtree.GetIndexFromQuad(searchQuads[k], i), point, &angle);
                int quadrant = GetQuadrant(angle);

                decimal elevation = lineStrips->GetStripElevation(quadtree.GetIndexFromQuad(searchQuads[k], i).stripIdx);
                if (lineDist < 1e-7)
                {
                    return elevation;
                }

                pointTotal[quadrant]++;
                decimal inverseDistance = 1.0 / (lineDist);
                totalElevations[quadrant] += (inverseDistance * elevation);
                totalInverseDistances[quadrant] += inverseDistance;

                foundAPoint = true;
            }
        }

        // Determine if we have points in opposing quadrants by using the index with the most points as the dividing line.
        int indexWithMostPoints = 0;
        int mostPoints = 0;
        for (int i = 0; i < quadrants; i++)
        {
            if (pointTotal[i] > mostPoints)
            {
                mostPoints = pointTotal[i];
                indexWithMostPoints = i;
            }
        }

        bool pointsInOpposingQuadrants = pointTotal[(indexWithMostPoints + 3) % quadrants] != 0 || pointTotal[(indexWithMostPoints + 4) % quadrants] != 0 || pointTotal[(indexWithMostPoints + 5) % quadrants] != 0;
        if (foundAPoint && (overrun >= overrunLimit || pointsInOpposingQuadrants))
        {
            decimal elevation = 0;
            int quadrantsWithPoints = 0;
            for (int i = 0; i < quadrants; i++)
            {
                if (pointTotal[i] != 0)
                {
                    elevation += totalElevations[i] / (totalInverseDistances[i]);
                    ++quadrantsWithPoints;
                }
            }

            return elevation / quadrantsWithPoints;
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

                decimal angle;
                decimal lineDist = GetLineDistanceWithAngle(index, point, &angle);
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

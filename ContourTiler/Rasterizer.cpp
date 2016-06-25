#include <cmath>
#include <iostream>
#include <limits>
#include "Rasterizer.h"

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader, int size)
    : lineStrips(lineStripLoader), size(size)
{
}

// True if the line intersects the given quad, false otherwise.
bool Rasterizer::DoesLineIntersect(sf::Vector2<double> start, sf::Vector2<double> end, sf::Rect<double> quadArea)
{
    // x = start_x + (end_x - start_x)*t
    
    // Check the left and right.
    double leftX = (quadArea.left - start.x) / (end.x - start.x);
    if (leftX > 0 && leftX < 1)
    {
        // Is the y-coordinate within bounds then? Return true.
        double yPos = leftX * (end.y - start.y) + start.y;
        if (yPos > quadArea.top && yPos < (quadArea.top + quadArea.width))
        {
            return true;
        }
    }

    double rightX = ((quadArea.left + quadArea.width) - start.x) / (end.x - start.x);
    if (rightX > 0 && rightX < 1)
    {
        // Is the y-coordinate within bounds then? Return true.
        double yPos = rightX * (end.y - start.y) + start.y;
        if (yPos > quadArea.top && yPos < (quadArea.top + quadArea.width))
        {
            return true;
        }
    }

    // Same business with the upper and lower portions.
    double lowerY = (quadArea.top - start.y) / (end.y - start.y);
    if (lowerY > 0 && lowerY < 1)
    {
        double xPos = lowerY * (end.x - start.x) + start.x;
        if (xPos > quadArea.left && xPos < (quadArea.left + quadArea.width))
        {
            return true;
        }
    }

    double upperY = ((quadArea.top + quadArea.height) - start.y) / (end.y - start.y);
    if (upperY < 0 || upperY > 1)
    {
        double xPos = upperY * (end.x - start.x) + start.x;
        if (xPos > quadArea.left && xPos < (quadArea.left + quadArea.width))
        {
            return true;
        }
    }

    return false;
}

sf::Vector2i Rasterizer::GetQuadtreeSquare(Point givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * size), size - 1),
        std::min((int)(givenPoint.y * size), size - 1));
}

void Rasterizer::Setup()
{
    // Fill in the quadtree with the empty index vector lists.
    quadtree.clear();
    for (int i = 0; i < size * size; i++)
    {
        std::vector<Index> emptyVector;
        quadtree.push_back(emptyVector);
    }

    // Now fill in all the vector lists with the indexes of all the lines within the area.
    for (int i = 0; i < lineStrips->GetStripCount(); i++)
    {
        std::vector<Point> points;
        lineStrips->ReadAllPoints(i, points);

        for (unsigned int j = 0; j < points.size() - 1; j++)
        {
            sf::Vector2i lineStart = GetQuadtreeSquare(points[j]);
            sf::Vector2i lineEnd = GetQuadtreeSquare(points[j + 1]);

            // v1. Iterate through the square of squares containing the start and end to determine if the line is within those quadtrees.
            sf::Vector2i minArea = sf::Vector2i(
                std::min(lineStart.x, lineEnd.x),
                std::min(lineStart.y, lineEnd.y));

            sf::Vector2i maxArea = sf::Vector2i(
                std::max(lineStart.x, lineEnd.x),
                std::max(lineStart.y, lineEnd.y));

            Index index;

            index.stripIdx = i;
            index.pointIdx = j;
            quadtree[lineStart.x + lineStart.y * size].push_back(index);
            quadtree[lineEnd.x + lineEnd.y * size].push_back(index);

            /*for (int m = minArea.x; m <= maxArea.x; m++)
            {
                for (int n = minArea.y; n <= maxArea.y; n++)
                {
                    if (DoesLineIntersect(start, end, sf::Rect<double>((double)m / (double)size, (double)n / (double)size, 1.0 / (double)size, 1.0 / (double)size)))
                    {
                        Index index;

                        index.stripIdx = i;
                        index.pointIdx = j;
                        quadtree[m + n * size].push_back(index);
                    }
                }
            }*/
        }

        if (i % 100 == 0)
        {
            std::cout << "Added strip " << i << " of " << lineStrips->GetStripCount() << " to the quadtree." << std::endl;
        }
    }
}

// Same as the above but treats the index as a line.
double Rasterizer::GetLineDistance(Index idx, Point point)
{
    Point startGiven = lineStrips->ReadPoint(idx);
    idx.pointIdx++;
    Point endGiven = lineStrips->ReadPoint(idx);

    sf::Vector2<double> start(startGiven.x, startGiven.y);
    sf::Vector2<double> end(endGiven.x, endGiven.y);
    sf::Vector2<double> length = end - start;
    // ax + by + c = 0 form
    double a = length.x;
    double b = -length.y;
    double c = (start.y * length.y - start.x * length.x);
    
    double distanceSqd = abs(a * start.x + b * start.y + c) / sqrt(a * a + b * b);

    sf::Vector2<double> intersectionPoint(
        (b * (b * start.x - a * start.y) - (a * c)) / (a * a + b * b),
        (a * (a * start.y - b * start.x) - (b * c)) / (a * a + b * b));
    if (intersectionPoint.x < std::min(start.x, end.x) || intersectionPoint.x > std::max(start.x, end.x) ||
        intersectionPoint.y < std::min(start.y, end.y) || intersectionPoint.y > std::max(start.y, end.y))
    {
        // Find the minimum distance from the start and end and return that instead.
        return std::min(
            sqrt(pow(start.x - point.x, 2) + pow(start.y - point.y, 2)),
            sqrt(pow(end.x - point.x, 2) + pow(end.y - point.y, 2)));
    }

    return distanceSqd;
}

void Rasterizer::AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    if (xP >= 0 && yP >= 0 && xP < size && yP < size)
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

double Rasterizer::FindClosestPoint(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);
    
    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 0;
    std::vector<sf::Vector2i> searchQuads;
    while (true)
    {
        searchQuads.clear();
        AddAreasToSearch(gridDistance, quadSquare, searchQuads);

        double distance = std::numeric_limits<double>::max();
        bool foundAPoint = false;
        Index selectedIndex;
        for (int k = 0; k < searchQuads.size(); k++)
        {
            int quadIdx = searchQuads[k].x + searchQuads[k].y * size;
            for (int i = 0; i < quadtree[quadIdx].size(); i++)
            {
                double lineDist = GetLineDistance(quadtree[quadIdx][i], point);
                if (lineDist < distance)
                {
                    distance = lineDist;
                    selectedIndex = quadtree[quadIdx][i];
                }

                foundAPoint = true;
            }
        }

        if (foundAPoint)
        {
            return lineStrips->GetStripElevation(selectedIndex.stripIdx);
        }

        ++gridDistance;
    }
}

void Rasterizer::Rasterize(sf::Rect<double> boundingBox, double** rasterStore, double& minElevation, double& maxElevation)
{
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();
    
    std::cout << "Rasterizing..." << std::endl;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            double x = (boundingBox.left + ((double)i / (double)size) * boundingBox.width);
            double y = (boundingBox.top + ((double)j / (double)size) * boundingBox.height);

            Point point(x, y);
            double elevation = FindClosestPoint(point);
            (*rasterStore)[i + j * size] = elevation;

            if (elevation < minElevation)
            {
                minElevation = elevation;
            }

            if (elevation > maxElevation)
            {
                maxElevation = elevation;
            }
        }

        if (i % 20 == 0)
        {
            std::cout << "Rasterized line " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

void Rasterizer::LineRaster(sf::Rect<double> boundingBox, double** rasterStore)
{
    std::cout << "Line Rasterizing..." << std::endl;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            double x = (boundingBox.left + ((double)i / (double)size) * boundingBox.width);
            double y = (boundingBox.top + ((double)j / (double)size) * boundingBox.height);
            double wiggleDist = ((double)1 / (double)size) * boundingBox.width;

            Point point(x, y);

            sf::Vector2i quadSquare = GetQuadtreeSquare(point);
            int quadIdx = quadSquare.x + quadSquare.y * size;
            for (int k = 0; k < quadtree[quadIdx].size(); k++)
            {
                double lineDist = GetLineDistance(quadtree[quadIdx][k], point);
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

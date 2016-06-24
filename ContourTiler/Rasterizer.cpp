#include <cmath>
#include <iostream>
#include <limits>
#include "Rasterizer.h"

Rasterizer::Rasterizer(int divisionFactor)
    : divisionFactor(divisionFactor)
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

    return true;
}

sf::Vector2i Rasterizer::GetQuadtreeSquare(sf::Vector2<double> givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * divisionFactor), divisionFactor - 1),
        std::min((int)(givenPoint.y * divisionFactor), divisionFactor - 1));
}

void Rasterizer::Setup(std::vector<LineStrip>& lineStrips)
{
    // Fill in the quadtree with the empty index vector lists.
    quadtree.clear();
    for (int i = 0; i < divisionFactor * divisionFactor; i++)
    {
        std::vector<Index> emptyVector;
        quadtree.push_back(emptyVector);
    }

    // Now fill in all the vector lists with the indexes of all the lines within the area.
    for (unsigned int i = 0; i < lineStrips.size(); i++)
    {
        for (unsigned int j = 0; j < lineStrips[i].points.size() - 1; j++)
        {
            Point startP = lineStrips[i].points[j];
            Point endP = lineStrips[i].points[j + 1];
            sf::Vector2<double> start((double)startP.x / (double)std::numeric_limits<unsigned short>::max(), (double)startP.y / (double)std::numeric_limits<unsigned short>::max());
            sf::Vector2<double> end((double)endP.x / (double)std::numeric_limits<unsigned short>::max(), (double)endP.y / (double)std::numeric_limits<unsigned short>::max());

            sf::Vector2i lineStart = GetQuadtreeSquare(start);
            sf::Vector2i lineEnd = GetQuadtreeSquare(end);

            // v1. Iterate through the square of squares containing the start and end to determine if the line is within those quadtrees.
            sf::Vector2i minArea = sf::Vector2i(
                std::min(lineStart.x, lineEnd.x),
                std::min(lineStart.y, lineEnd.y));

            sf::Vector2i maxArea = sf::Vector2i(
                std::max(lineStart.x, lineEnd.x),
                std::max(lineStart.y, lineEnd.y));

            for (int m = minArea.x; m <= maxArea.x; m++)
            {
                for (int n = minArea.y; n <= maxArea.y; n++)
                {
                    if (DoesLineIntersect(start, end, sf::Rect<double>((double)m / (double)divisionFactor, (double)n / (double)divisionFactor, 1.0 / (double)divisionFactor, 1.0 / (double)divisionFactor)))
                    {
                        Index index;

                        index.stripIdx = i;
                        index.pointIdx = j;
                        quadtree[m + n * divisionFactor].push_back(index);
                    }
                }
            }
        }

        if (i % 100 == 0)
        {
            std::cout << "Added strip " << i << " of " << lineStrips.size() << " to the quadtree." << std::endl;
        }
    }
}

// Same as the above but treats the index as a line.
double Rasterizer::GetLineDistance(std::vector<LineStrip>& lineStrips, Index idx, sf::Vector2<double> point)
{
    sf::Vector2<double> start((double)lineStrips[idx.stripIdx].points[idx.pointIdx].x / (double)std::numeric_limits<unsigned short>::max(), (double)lineStrips[idx.stripIdx].points[idx.pointIdx].y / (double)std::numeric_limits<unsigned short>::max());
    sf::Vector2<double> end((double)lineStrips[idx.stripIdx].points[idx.pointIdx + 1].x / (double)std::numeric_limits<unsigned short>::max(), (double)lineStrips[idx.stripIdx].points[idx.pointIdx + 1].y / (double)std::numeric_limits<unsigned short>::max());
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
    if (xP >= 0 && yP >= 0 && xP < divisionFactor && yP < divisionFactor)
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

double Rasterizer::FindClosestPoint(std::vector<LineStrip>& lineStrips, sf::Vector2<double> point)
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
            int quadIdx = searchQuads[k].x + searchQuads[k].y * divisionFactor;
            for (int i = 0; i < quadtree[quadIdx].size(); i++)
            {
                double lineDist = GetLineDistance(lineStrips, quadtree[quadIdx][i], point);
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
            return (double)lineStrips[selectedIndex.stripIdx].elevation / (double)std::numeric_limits<unsigned short>::max();;
        }

        ++gridDistance;
    }
}

void Rasterizer::Rasterize(std::vector<LineStrip>& lineStrips, sf::Rect<double> boundingBox, int width, int height, double** rasterStore, double& minElevation, double& maxElevation)
{
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();
    
    std::cout << "Rasterizing..." << std::endl;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = (boundingBox.left + ((double)i / (double)width) * boundingBox.width);
            double y = (boundingBox.top + ((double)j / (double)height) * boundingBox.height);

            sf::Vector2<double> point(x, y);

            double elevation = FindClosestPoint(lineStrips, point);
            (*rasterStore)[i + j * width] = elevation;

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
            std::cout << "Rasterized line " << i << " of " << width << std::endl;
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

void Rasterizer::LineRaster(std::vector<LineStrip>& lineStrips, sf::Rect<double> boundingBox, int width, int height, double** rasterStore)
{
    std::cout << "Line Rasterizing..." << std::endl;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            sf::Vector2<double> point;
            point.x = (boundingBox.left + ((double)i / (double)width) * boundingBox.width);
            point.y = (boundingBox.top + ((double)j / (double)height) * boundingBox.height);
            double wiggleDist = ((double)1 / (double)width) * boundingBox.width;

            sf::Vector2i quadSquare = GetQuadtreeSquare(point);
            int quadIdx = quadSquare.x + quadSquare.y * divisionFactor;
            for (int k = 0; k < quadtree[quadIdx].size(); k++)
            {
                double lineDist = GetLineDistance(lineStrips, quadtree[quadIdx][k], point);
                if (lineDist < wiggleDist)
                {
                    (*rasterStore)[i + j * width] = 2e8; // Bigger than anything we'll hit.
                    break;
                }
            }

        }

        if (i % 20 == 0)
        {
            std::cout << "Line rasterized line " << i << " of " << width << std::endl;
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

Rasterizer::~Rasterizer()
{
}

#include <cmath>
#include <iostream>
#include <limits>
#include "Rasterizer.h"

Rasterizer::Rasterizer(double xMax, double yMax, int divisionFactor)
    : xMax(xMax), yMax(yMax), divisionFactor(divisionFactor)
{
}

// True if the line intersects th given quad, false otherwise.
bool Rasterizer::DoesLineIntersect(sf::Vector2<double> start, sf::Vector2<double> end, sf::Rect<double> quadArea)
{
    // TODO optimize this substantially.
    return true;
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
            sf::Vector2<double> start((lineStrips[i].points[j].x / xMax), (lineStrips[i].points[j].y / yMax));
            sf::Vector2<double> end((lineStrips[i].points[j + 1].x / xMax), (lineStrips[i].points[j + 1].y / yMax));

            // Starting quadtree
            int xPS = std::min((int)(start.x * divisionFactor), divisionFactor - 1);
            int yPS = std::min((int)(start.y * divisionFactor), divisionFactor - 1);
            
            int xPE = std::min((int)(end.x * divisionFactor), divisionFactor - 1);
            int yPE = std::min((int)(end.y * divisionFactor), divisionFactor - 1);
            
            // v1. Iterate through the square of squares containing the start and end to determine if the line is within those quadtrees.
            int xMin = std::min(xPS, xPE);
            int yMin = std::min(yPS, yPE);
            int xMax = std::max(xPS, xPE);
            int yMax = std::max(yPS, yPE);
            for (int m = xMin; m <= xMax; m++)
            {
                for (int n = yMin; n <= yMax; n++)
                {
                    if (DoesLineIntersect(start, end, sf::Rect<double>((double)m / divisionFactor, (double)n / divisionFactor, 1.0 / (double)divisionFactor, 1.0 / (double)divisionFactor)))
                    {
                        Index index;

                        index.stripIdx = i;
                        index.pointIdx = j;
                        quadtree[m + n * divisionFactor].push_back(index);
                    }
                }
            }
        }

        if (i % 10000 == 0)
        {
            std::cout << "Added strip " << i << " of " << lineStrips.size() << " to the quadtree." << std::endl;
        }
    }
}

double Rasterizer::GetDistance(std::vector<LineStrip>& lineStrips, Index idx, double x, double y)
{
    return pow(lineStrips[idx.stripIdx].points[idx.pointIdx].x - x, 2) + pow(lineStrips[idx.stripIdx].points[idx.pointIdx].y - y, 2);
}

// Same as the above but treats the index as a line.
double Rasterizer::GetLineDistance(std::vector<LineStrip>& lineStrips, Index idx, double x, double y)
{
    sf::Vector2<double> point(x, y);
    sf::Vector2<double> start(lineStrips[idx.stripIdx].points[idx.pointIdx].x, lineStrips[idx.stripIdx].points[idx.pointIdx].y);
    sf::Vector2<double> end(lineStrips[idx.stripIdx].points[idx.pointIdx + 1].x, lineStrips[idx.stripIdx].points[idx.pointIdx + 1].y);
    sf::Vector2<double> n = end - start;
    sf::Vector2<double> startMinusPoint = start - point;

    sf::Vector2<double> lengthVector = startMinusPoint - (startMinusPoint.x * n.x + startMinusPoint.y * n.y) * n;
    return lengthVector.x*lengthVector.x + lengthVector.y * lengthVector.y;
}

Point Rasterizer::GetPoint(std::vector<LineStrip>& lineStrips, std::vector<Index>::iterator iter)
{
    return lineStrips[iter->stripIdx].points[iter->pointIdx];
}

void Rasterizer::AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    if (xP >= 0 && yP >= 0 && xP < divisionFactor && yP < divisionFactor)
    {
        searchQuads.push_back(sf::Vector2i(xP, yP));
    }
}

void Rasterizer::AddAreasToSearch(int distance, int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    if (distance == 0)
    {
        searchQuads.push_back(sf::Vector2i(xP, yP));
    }
    else
    {
        // Add the horizontal bars.
        for (int i = xP - distance; i <= xP + distance; i++)
        {
            AddIfValid(i, yP + distance, searchQuads);
            AddIfValid(i, yP - distance, searchQuads);
        }

        // Add the vertical bars, skipping the corners that otherwise would be duplicated.
        for (int j = yP - (distance - 1); j <= yP + (distance - 1); j++)
        {
            AddIfValid(xP + distance, j, searchQuads);
            AddIfValid(xP - distance, j, searchQuads);
        }
    }
}

double Rasterizer::FindClosestPoint(std::vector<LineStrip>& lineStrips, double x, double y)
{
    int xP = std::min((int)((x / xMax) * divisionFactor), divisionFactor - 1);
    int yP = std::min((int)((y / yMax) * divisionFactor), divisionFactor - 1);
    
    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 0;
    std::vector<sf::Vector2i> searchQuads;
    while (true)
    {
        searchQuads.clear();
        AddAreasToSearch(gridDistance, xP, yP, searchQuads);

        double distance = std::numeric_limits<double>::max();
        bool foundAPoint = false;
        Index selectedIndex;
        for (int k = 0; k < searchQuads.size(); k++)
        {
            int quadIdx = searchQuads[k].x + searchQuads[k].y * divisionFactor;
            for (int i = 0; i < quadtree[quadIdx].size(); i++)
            {
                // double pointDist = GetDistance(quadtree[quadIdx][i], x, y);
                double lineDist = GetLineDistance(lineStrips, quadtree[quadIdx][i], x, y);
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
            return lineStrips[selectedIndex.stripIdx].elevation;
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
            double x = (boundingBox.left + ((double)i / (double)width) * boundingBox.width) * xMax;
            double y = yMax - (boundingBox.top + ((double)j / (double)height) * boundingBox.height) * yMax;

            double elevation = FindClosestPoint(lineStrips, x, y);
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

        if (i % 10 == 0)
        {
            std::cout << "Rasterized line " << i << " of " << width << std::endl;
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

Rasterizer::~Rasterizer()
{
}

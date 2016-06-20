#include <cmath>
#include <iostream>
#include <limits>
#include "Rasterizer.h"

Rasterizer::Rasterizer(double xMax, double yMax, int divisionFactor)
    : xMax(xMax), yMax(yMax), divisionFactor(divisionFactor)
{
}

void Rasterizer::Setup(std::vector<LineStrip>* lineStrips)
{
    this->lineStrips = lineStrips;

    // Fill in the quadtree with the empty index vector lists.
    quadtree.clear();
    for (int i = 0; i < divisionFactor * divisionFactor; i++)
    {
        std::vector<Index> emptyVector;
        quadtree.push_back(emptyVector);
    }

    // Now fill in all the vector lists with the indexes of 
    for (unsigned int i = 0; i < lineStrips->size(); i++)
    {
        for (unsigned int j = 0; j < (*lineStrips)[i].points.size(); j++)
        {
            int xP = std::min((int)(((*lineStrips)[i].points[j].x / xMax) * divisionFactor), divisionFactor - 1);
            int yP = std::min((int)(((*lineStrips)[i].points[j].y / yMax) * divisionFactor), divisionFactor - 1);
            
            Index index;
            index.stripIdx = i;
            index.pointIdx = j;
            quadtree[xP + yP * divisionFactor].push_back(index);
        }

        if (i % 10000 == 0)
        {
            std::cout << "Added strip " << i << " of " << lineStrips->size() << " to the quadtree." << std::endl;
        }
    }
}

double Rasterizer::GetDistance(Index idx, double x, double y)
{
    return pow((*lineStrips)[idx.stripIdx].points[idx.pointIdx].x - x, 2) + pow((*lineStrips)[idx.stripIdx].points[idx.pointIdx].y - y, 2);
}

Point Rasterizer::GetPoint(std::vector<Index>::iterator iter)
{
    return (*lineStrips)[iter->stripIdx].points[iter->pointIdx];
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

double Rasterizer::FindClosestPoint(double x, double y)
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
                double pointDist = GetDistance(quadtree[quadIdx][i], x, y);
                if (pointDist < distance)
                {
                    distance = pointDist;
                    selectedIndex = quadtree[quadIdx][i];
                }

                foundAPoint = true;
            }
        }

        if (foundAPoint)
        {
            return (*lineStrips)[selectedIndex.stripIdx].elevation;
        }

        ++gridDistance;
    }
}

void Rasterizer::Rasterize(sf::Rect<double> boundingBox, int width, int height, double** rasterStore, double& minElevation, double& maxElevation)
{
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();
    
    std::cout << "Rasterizing..." << std::endl;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = boundingBox.left + ((double)i / (double)width) * boundingBox.width * xMax;
            double y = yMax - (boundingBox.top + ((double)j / (double)height)) * boundingBox.height * yMax;

            double elevation = FindClosestPoint(x, y);
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

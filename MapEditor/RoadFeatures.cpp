#include <iostream>
#include <fstream>
#include "RoadFeatures.h"



RoadFeatures::RoadFeatures()
{
}

bool RoadFeatures::LoadPointFeatures(std::string filename, std::vector<Point>& pointStorage)
{
    std::ifstream readFile(filename, std::ios::in | std::ios::binary);
    if (!readFile)
    {
        std::cout << "Could not load the " << filename << " file." << std::endl;
        return false;
    }

    int points;
    readFile.read((char*)&points, sizeof(int));
    std::cout << filename << ": Total of " << points << std::endl;
    for (int i = 0; i < points; i++)
    {
        Point point;
        readFile.read((char*)&point.x, sizeof(double));
        readFile.read((char*)&point.y, sizeof(double));

        pointStorage.push_back(point);
    }

    readFile.close();
    return true;
}

bool RoadFeatures::LoadRoadFeatures()
{
    // Load the points features
    if (!LoadPointFeatures("barricade.bin", barricades) ||
        !LoadPointFeatures("stops.bin", stops) ||
        !LoadPointFeatures("emitter.bin", emitters) ||
        !LoadPointFeatures("freewayModifier.bin", freewayModifiers))
    {
        return false;
    }

    // Load the streets.
    std::ifstream streetFile("streets.bin", std::ios::in | std::ios::binary);
    if (!streetFile)
    {
        std::cout << "Could not load the street file." << std::endl;
        return false;
    }

    int types;
    streetFile.read((char*)&types, sizeof(int));
    for (int i = 0; i < types; i++)
    {
        std::vector<LineStrip> lineStrips;

        int linesInType;
        streetFile.read((char*)&linesInType, sizeof(int));
        std::cout << i << ": Lines for type: " << linesInType << std::endl;
        int totalPoints = 0;

        for (int j = 0; j < linesInType; j++)
        {
            LineStrip lineStrip;

            int pointsInLine;
            streetFile.read((char*)&pointsInLine, sizeof(int));
            for (int k = 0; k < pointsInLine; k++)
            {
                Point point;
                streetFile.read((char*)&point.x, sizeof(double));
                streetFile.read((char*)&point.y, sizeof(double));

                lineStrip.points.push_back(point);
            }

            totalPoints += pointsInLine;
            lineStrips.push_back(lineStrip);
        }

        std::cout << i << ": Total points for type: " << totalPoints << std::endl;

        roads[(RoadSizes)i] = lineStrips;
    }
    std::cout << "Types: " << types << std::endl;
    
    streetFile.close();
    return true;
}
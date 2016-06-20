#include <iostream>
#include <fstream>
#include "LineStripLoader.h"

bool LineStripLoader::LoadLineStrips(std::string fileName, std::vector<LineStrip>* lineStrips)
{
    std::ifstream contourReader(fileName, std::ios::in | std::ios::binary);
    if (!contourReader)
    {
        std::cout << "Could not open the file to read contours from!" << std::endl;
        return false;
    }

    int lineStripCount;
    contourReader.read((char*)&lineStripCount, sizeof(int));
    std::cout << "Line strips: " << lineStripCount << std::endl;

    for (int i = 0; i < lineStripCount; i++)
    {
        LineStrip lineStrip;

        contourReader.read((char*)&lineStrip.elevation, sizeof(double));

        int pointCount;
        contourReader.read((char*)&pointCount, sizeof(int));

        for (int j = 0; j < pointCount; j++)
        {
            Point point;
            contourReader.read((char*)&point, sizeof(Point));
            lineStrip.points.push_back(point);
        }

        lineStrips->push_back(lineStrip);
        
        if (i % 10000 == 0)
        {
            std::cout << "  Read line strip " << i << " of " << lineStripCount << std::endl;
        }
    }

    return true;
}

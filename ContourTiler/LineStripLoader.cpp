#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <thread>
#include <map>
#include "LineStripLoader.h"

LineStripLoader::LineStripLoader()
{

}

bool LineStripLoader::Initialize(std::string lineStripFilename)
{
    std::ifstream lineStripFile(lineStripFilename, std::ios::in | std::ios::binary);
    if (!lineStripFile)
    {
        std::cout << "Could not open the file to read contours from!" << std::endl;
        return false;
    }

    int lineStripCount;
    lineStripFile.read((char*)&lineStripCount, sizeof(int));
    std::cout << "Line strips: " << lineStripCount << std::endl;

    lineStripCount = 300000; // Override to limit load time for rasterization changes.

    // Reported settings.
    double xMinRep, xMaxRep, yMinRep, yMaxRep, minERep, maxERep;
    lineStripFile.read((char*)&xMinRep, sizeof(double));
    lineStripFile.read((char*)&xMaxRep, sizeof(double));
    lineStripFile.read((char*)&yMinRep, sizeof(double));
    lineStripFile.read((char*)&yMaxRep, sizeof(double));
    lineStripFile.read((char*)&minERep, sizeof(double));
    lineStripFile.read((char*)&maxERep, sizeof(double));

    std::cout << "Limits reported to be [" << xMinRep << ", " << xMaxRep << "; " << yMinRep << ", " << yMaxRep << "; " << minERep << ", " << maxERep << "]." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Line strips now follow sequentially with the following format:
    //   double elevation;
    //   int pointCount;
    //   Point points[pointCount];
    lineStrips.clear();
    lineStrips.reserve(lineStripCount);
    long points = 0;

    std::map<int, int> elevationMap;
    for (int i = 0; i < lineStripCount; i++)
    {
        lineStrips.push_back(LineStrip());
        
        double elevation;
        lineStripFile.read((char*)&elevation, sizeof(double));
        if (elevationMap.find((int)elevation) == elevationMap.end())
        {
            int size = (int)elevationMap.size();
            lineStrips[i].elevationId = size;
            elevationMap[(int)elevation] = size;
        }
        else
        {
            lineStrips[i].elevationId = elevationMap[(int)elevation];
        }

        lineStrips[i].elevation = (elevation - Constant::ZMin) / (Constant::ZMax - Constant::ZMin);

        int pointCount;
        lineStripFile.read((char*)&pointCount, sizeof(int));

        int actualPointCount = pointCount;
        if (pointCount > 100)
        {
            pointCount = 100; // Override to speed up loading yet keep a reasonable overview.
        }

        lineStrips[i].points.reserve(pointCount);

        // Read in all the data, but don't process the bulk of it for quick exclusion tests.
        double* allPoints = new double[actualPointCount * 2];
        lineStripFile.read((char*)allPoints, sizeof(double) * 2 * actualPointCount);

        for (int j = 0; j < pointCount; j++)
        {
            ++points;
            if (points % 1000000 == 0)
            {
                std::cout << "Read in point " << points << ". (" << (float)points / 1.5e7f << "%)" << std::endl;
            }

            Point point;
            point.x = (decimal)((allPoints[j * 2] - Constant::XMin) / (Constant::XMax - Constant::XMin));
            point.y = (decimal)((allPoints[j * 2 + 1] - Constant::YMin) / (Constant::YMax - Constant::YMin));
            lineStrips[i].points.push_back(point);
        }

        delete[] allPoints;
        if (i % 10000 == 0)
        {
            std::cout << "Read in line strip " << i << std::endl;
        }
    }

    return true;
}

LineStripLoader::~LineStripLoader()
{
}
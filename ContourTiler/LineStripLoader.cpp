#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <thread>
#include "LineStripLoader.h"

bool LineStripLoader::LoadLineStrips(std::string fileName, std::vector<LineStrip>* lineStrips)
{
        /*std::ofstream dataFile("smallData.bin", std::ios::out | std::ios::binary);
        if (!dataFile)
        {
            std::cout << "Could not open the file to write the data to!" << std::endl;
            return false;
        }*/

    std::ifstream contourReader(fileName, std::ios::in | std::ios::binary);
    if (!contourReader)
    {
        std::cout << "Could not open the file to read contours from!" << std::endl;
        return false;
    }

    int lineStripCount;
    contourReader.read((char*)&lineStripCount, sizeof(int));
        //dataFile.write((char*)&lineStripCount, sizeof(int));
    std::cout << "Line strips: " << lineStripCount << std::endl;

    // Reported settings.
    /*double xMinRep, xMaxRep, yMinRep, yMaxRep, minERep, maxERep;
    contourReader.read((char*)&xMinRep, sizeof(double));
    contourReader.read((char*)&xMaxRep, sizeof(double));
    contourReader.read((char*)&yMinRep, sizeof(double));
    contourReader.read((char*)&yMaxRep, sizeof(double));
    contourReader.read((char*)&minERep, sizeof(double));
    contourReader.read((char*)&maxERep, sizeof(double));

    std::cout << "Limits reported to be [" << xMinRep << ", " << xMaxRep << "; " << yMinRep << ", " << yMaxRep << "; " << minERep << ", " << maxERep << "]." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    double xMin = 1214976.7500000000;
    double xMax = 1583489.0000000000;

    double yMin = 22489.217250004411;
    double yMax = 360015.64550000429;
    double minElevation = -900.00000000000000;
    double maxElevation = 7960.0000000000000;
    */
    long totalPointCount = 0;

    lineStrips->reserve(lineStripCount);
    for (int i = 0; i < lineStripCount; i++)
    {
        LineStrip lineStrip;

        contourReader.read((char*)&lineStrip.elevation, sizeof(unsigned short));
        /*double elevation;
        contourReader.read((char*)&elevation, sizeof(double));
        lineStrip.elevation = (unsigned short)((elevation - minElevation / (maxElevation - minElevation)) * (double)std::numeric_limits<unsigned short>::max());
            dataFile.write((char*)&lineStrip.elevation, sizeof(unsigned short));*/
     //   unsigned short unusedForAlignment;
     //   contourReader.read((char*)&unusedForAlignment, sizeof(unsigned short));

        int pointCount;
        contourReader.read((char*)&pointCount, sizeof(int));
            //dataFile.write((char*)&pointCount, sizeof(int));

        lineStrip.points.reserve(pointCount);
        for (int j = 0; j < pointCount; j++)
        {
            Point point;
            contourReader.read((char*)&point, sizeof(Point));
            lineStrip.points.push_back(point);
            /*
            double xy[2];
            contourReader.read((char*)&xy, sizeof(double) * 2);

            // Rescale x and y
            Point point;
            point.x = (unsigned short)((xy[0] - xMin / (xMax - xMin)) * (double)std::numeric_limits<unsigned short>::max());
            point.y = (unsigned short)((xy[1] - yMin / (yMax - yMin)) * (double)std::numeric_limits<unsigned short>::max());
            lineStrip.points.push_back(point);
                dataFile.write((char*)&point.x, sizeof(unsigned short));
                dataFile.write((char*)&point.y, sizeof(unsigned short));*/
        }

        if (lineStrip.points.size() != 0)
        {
            lineStrips->push_back(lineStrip);
        }
        else
        {
            std::cout << "Zero points found at index " << i << "." << std::endl;
        }
        
        if (i % 1000 == 0)
        {
            std::cout << "  Read line strip " << i << " of " << lineStripCount << std::endl;
        }

        totalPointCount += pointCount;
    }

    //dataFile.close();

    std::wcout << "Total point count: " << totalPointCount << std::endl;
    //std::cout << "Limits reported to be [" << xMinRep << ", " << xMaxRep << "; " << yMinRep << ", " << yMaxRep << "; " << minERep << ", " << maxERep << "]." << std::endl;
    return true;
}

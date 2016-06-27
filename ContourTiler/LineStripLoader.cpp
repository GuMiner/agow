#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <thread>
#include "LineStripLoader.h"

LineStripLoader::LineStripLoader() : lineStripFile(nullptr)
{

}

bool LineStripLoader::LoadIndices(std::string indexFilename, int lineStripCount)
{
    // Read in all the indices of all line strips
    std::ifstream indexFile(indexFilename, std::ios::in | std::ios::binary);
    if (!lineStripFile)
    {
        std::cout << "Could not open the file to read contour indices from!" << std::endl;
        return false;
    }

    int readLineStripCount = 0;
    indexFile.read((char*)&readLineStripCount, sizeof(int));
    std::cout << "Index reported line strips: " << lineStripCount << std::endl;

    if (lineStripCount != readLineStripCount)
    {
        std::cout << "Mismatching index file! The line strip count was " << readLineStripCount << " and not " << lineStripCount << std::endl;
        // indexFile.close();
        // return false;
    }

    indexPositions.reserve(lineStripCount);
    for (int i = 0; i < lineStripCount; i++)
    {
        std::streampos position;
        indexFile.read((char*)&position, sizeof(std::streampos));
        indexPositions.push_back(position);

        if (i % 25000 == 0)
        {
            std::cout << "  Read index " << i << " of " << lineStripCount << std::endl;
        }
    }

    indexFile.close();
    return true;
}

bool LineStripLoader::LoadElevations(std::string elevationFilename)
{
    // Read in all the indices of all line strips
    std::ifstream elevationFile(elevationFilename, std::ios::in | std::ios::binary);
    if (!lineStripFile)
    {
        std::cout << "Could not open the file to read elevations from!" << std::endl;
        return false;
    }

    double minElevation = std::numeric_limits<double>::max();
    double maxElevation = std::numeric_limits<double>::min();
    indexElevations.reserve(indexPositions.size());
    for (int i = 0; i < indexPositions.size(); i++)
    {
        double elevation;
        elevationFile.read((char*)&elevation, sizeof(double));
        indexElevations.push_back(elevation);

        if (elevation > maxElevation)
        {
            maxElevation = elevation;
        }
        
        if (elevation < minElevation)
        {
            minElevation = elevation;
        }

        if (i % 25000 == 0)
        {
            std::cout << "  Read elevation " << i << " of " << indexPositions.size() << std::endl;
        }
    }

    std::cout << "Elevation range from " << minElevation << " to " << maxElevation << " read." << std::endl;
    elevationFile.close();
    return true;
}

bool LineStripLoader::InitializeFiles(std::string lineStripFilename, std::string indexFilename, std::string elevationFilename)
{
    lineStripFile = new std::ifstream(lineStripFilename, std::ios::in | std::ios::binary);
    if (!lineStripFile)
    {
        std::cout << "Could not open the file to read contours from!" << std::endl;
        return false;
    }

    int lineStripCount;
    lineStripFile->read((char*)&lineStripCount, sizeof(int));
    std::cout << "Line strips: " << lineStripCount << std::endl;

    lineStripCount = 300000; // Override to limit load time for rasterization changes.

    // Reported settings.
    double xMinRep, xMaxRep, yMinRep, yMaxRep, minERep, maxERep;
    lineStripFile->read((char*)&xMinRep, sizeof(double));
    lineStripFile->read((char*)&xMaxRep, sizeof(double));
    lineStripFile->read((char*)&yMinRep, sizeof(double));
    lineStripFile->read((char*)&yMaxRep, sizeof(double));
    lineStripFile->read((char*)&minERep, sizeof(double));
    lineStripFile->read((char*)&maxERep, sizeof(double));

    std::cout << "Limits reported to be [" << xMinRep << ", " << xMaxRep << "; " << yMinRep << ", " << yMaxRep << "; " << minERep << ", " << maxERep << "]." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Line strips now follow sequentially with the following format:
    //   double elevation;
    //   int pointCount;
    //   Point points[pointCount];
    lineStrips.clear();
    lineStrips.reserve(lineStripCount);
    long points = 0;
    for (int i = 0; i < lineStripCount; i++)
    {
        lineStrips.push_back(LineStrip());
        double unusedElevation;
        lineStripFile->read((char*)&unusedElevation, sizeof(double));

        int pointCount;
        lineStripFile->read((char*)&pointCount, sizeof(int));

        lineStrips[i].points.reserve(pointCount);
        for (int j = 0; j < pointCount; j++)
        {
            double xy[2];
            lineStripFile->read((char*)&xy, sizeof(double) * 2);

            Point point;
            point.x = (decimal)((xy[0] - Constant::XMin) / (Constant::XMax - Constant::XMin));
            point.y = (decimal)((xy[1] - Constant::YMin) / (Constant::YMax - Constant::YMin));
            lineStrips[i].points.push_back(point);

            ++points;
            if (points % 1000000 == 0)
            {
                std::cout << "Read in point " << points << ". ("  << (float)points / 1.5e7f << "%)" << std::endl;
            }
        }

        if (i % 10000 == 0)
        {
            std::cout << "Read in line strip " << i << std::endl;
        }
    }
    
    if (!LoadIndices(indexFilename, lineStripCount))
    {
        return false;
    }

    if (!LoadElevations(elevationFilename))
    {
        return false;
    }

    return true;
}

int LineStripLoader::GetStripCount() const
{
    return (int)indexPositions.size();
}

double LineStripLoader::GetStripElevation(const int stripIdx) const
{
    return (indexElevations[stripIdx] - Constant::ZMin) / (Constant::ZMax - Constant::ZMin);
}

int LineStripLoader::GetPointCount(const int stripIdx)
{
    int pointCount;
    lineStripFile->seekg((unsigned long long)indexPositions[stripIdx] + sizeof(double), std::ios::beg);
    lineStripFile->read((char*)&pointCount, sizeof(int));
    return pointCount;
}

Point LineStripLoader::ReadPoint(const Index index)
{
    Point point;
    lineStripFile->seekg((unsigned long long)indexPositions[index.stripIdx] + sizeof(double) + sizeof(int) + sizeof(double)*index.pointIdx, std::ios::beg);
    lineStripFile->read((char*)&point, sizeof(Point));

    point.x = (point.x - Constant::XMin) / (Constant::XMax - Constant::XMin);
    point.y = (point.y - Constant::YMin) / (Constant::YMax - Constant::YMin);
    return point;
}

void LineStripLoader::ReadAllPoints(const int stripIdx, std::vector<Point>& points)
{
    int pointCount = GetPointCount(stripIdx);

    // We are now right at the start of the points array, so read them all in.
    points.reserve(pointCount);
    for (int i = 0; i < pointCount; i++)
    {
        Point point;
        lineStripFile->read((char*)&point, sizeof(Point));

        point.x = (point.x - Constant::XMin) / (Constant::XMax - Constant::XMin);
        point.y = (point.y - Constant::YMin) / (Constant::YMax - Constant::YMin);
        points.push_back(point);
    }
}

LineStripLoader::~LineStripLoader()
{
    if (lineStripFile != nullptr)
    {
        lineStripFile->close();
        delete lineStripFile;
    }
}
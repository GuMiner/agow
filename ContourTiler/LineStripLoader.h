#pragma once
#include <string>
#include "Definitions.h"

class LineStripLoader
{
    std::ifstream* lineStripFile;
    

    std::vector<std::streampos> indexPositions;
    std::vector<double> indexElevations;

    bool LoadIndices(std::string indexFilename, int lineStripCount);
    bool LoadElevations(std::string elevationFilename);
public:
    LineStripLoader();

    std::vector<LineStrip> lineStrips;
    bool InitializeFiles(std::string lineStripFilename, std::string indexFilename, std::string elevationFilename);
    
    // Get basic details about the strips
    int GetStripCount() const;
    double GetStripElevation(const int stripIdx) const;
    int GetPointCount(const int stripIdx);

    // Acquire points in the strip. Points are normalized to 0-1 in all dimensions using the constants in Definitions.h
    Point ReadPoint(const Index index);
    void ReadAllPoints(const int stripIdx, std::vector<Point>& points);

    virtual ~LineStripLoader();
};


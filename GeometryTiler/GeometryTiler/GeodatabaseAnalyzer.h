#pragma once
#include <FileGDBAPI.h>
#include <vector>
#include "ErrorLogger.h"

struct Point
{
    double x, y;
};

struct LineStrip
{
    double elevation;
    std::vector<Point> points;
};

class GeodatabaseAnalyzer
{
    FileGDBAPI::Geodatabase geodatabase;
    std::wstring primaryDataset;
    bool dbOpened;

    FileGDBAPI::Table primaryTable;
    FileGDBAPI::EnumRows allRows;
    int rowCount, returnedRows;

    double minX, minY, maxX, maxY, minElevation, maxElevation;
    std::vector<LineStrip> lineStrips;

    void CheckLimits(Point point);
    long ProcessRow(FileGDBAPI::Row& row);

public:
    GeodatabaseAnalyzer(std::wstring databasePath, std::wstring primaryDataset);
    bool IsDbOpened() const;
    void Analyze(void) const;
    bool LoadTable();
    void AnalyzeTable(void) const;

    void LoadAllRows();
    void ProcessAllRows();
    void SaveSanitizedData(double maxX, double maxY, double maxZ);

    void UnloadTable();
    ~GeodatabaseAnalyzer(void);
};


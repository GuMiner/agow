#pragma once
#include <FileGDBAPI.h>

struct Dataset
{
    std::wstring name;
    FileGDBAPI::Table table;
    FileGDBAPI::EnumRows rows;
    int rowCount;

    Dataset(std::wstring name)
        : name(name)
    {
    }
};

struct SPoint
{
    double x, y;
};

struct SLineStrip
{
    std::wstring streetType;
    std::vector<SPoint> points;
};

class StreetDbAnalyzer
{
    bool dbOpened;
    
    FileGDBAPI::Geodatabase geodatabase;
    Dataset barricade;
    Dataset stops;
    Dataset streets;
    Dataset emitter;
    Dataset freewayModifier;

    double xMin;
    double xMax;
    double yMin;
    double yMax;

    void ReadDatasetType(std::wstring datasetName) const;
    void AnalyzeTable(std::string prefix, const FileGDBAPI::Table& table) const;

    bool LoadAllTableRows(Dataset& dataset);

public:
    StreetDbAnalyzer(std::wstring databasePath, 
                     std::wstring barricadeDataset,
                     std::wstring stopsDataset,
                     std::wstring streetsDataset, 
                     std::wstring emitterDataset,
                     std::wstring freewayModifierDataset);

    // Factors to rescale. Output is rescaled to 0 to 1 for X and Y.
    void SetRescaleFactors(double xMin, double xMax, double yMin, double yMax);
    void RescaleToFractions(double* x, double* y);

    bool IsDbOpened() const;
    void Analyze(void) const;
    bool LoadTables();
    void AnalyzeTables() const;

    void LoadAllRows();

    void SavePointRow(FileGDBAPI::Row& row, std::ofstream& dataFile);
    void ProcessBarricadeRows();
    void ProcessStopsRows();
    void ProcessStreetsRows();
    void ProcessEmitterRows();
    void ProcessFreewayModifierRows();

    void UnloadTables();

    ~StreetDbAnalyzer(void);
};


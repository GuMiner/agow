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

    // Factors to rescale. Output is rescaled to [0-70,000) for X and Y.
    void SetRescaleFactors(int xMin, int xMax, int yMin, int yMax);

    bool IsDbOpened() const;
    void Analyze(void) const;
    bool LoadTables();
    void AnalyzeTables() const;

    void LoadAllRows();
    void ProcessBarricadeRows();
    void ProcessStopsRows();
    void ProcessStreetsRows();
    void ProcessEmitterRows();
    void ProcessFreewayModifierRows();

    void UnloadTables();

    ~StreetDbAnalyzer(void);
};


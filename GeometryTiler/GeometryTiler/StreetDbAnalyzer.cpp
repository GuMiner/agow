#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>
#include <limits>
#include "ErrorLogger.h"
#include "StreetDbAnalyzer.h"


StreetDbAnalyzer::StreetDbAnalyzer(std::wstring databasePath, 
                                   std::wstring barricadeDataset,
                                   std::wstring stopsDataset,
                                   std::wstring streetsDataset, 
                                   std::wstring emitterDataset,
                                   std::wstring freewayModifierDataset)
                                   : dbOpened(false), barricade(barricadeDataset), stops(stopsDataset),
                                     streets(streetsDataset), emitter(emitterDataset), freewayModifier(freewayModifierDataset)
{
    fgdbError result;
    std::wcout << "Opening the db..." << std::endl;
    if ((result = FileGDBAPI::OpenGeodatabase(databasePath, geodatabase)) != S_OK)
    {
        ErrorLogger::LogError(L"Could not open the Geodatabase!", result);

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        return;
    }

    std::wcout << "DB opened." << std::endl;
    dbOpened = true;
}

void StreetDbAnalyzer::SetRescaleFactors(int xMin, int xMax, int yMin, int yMax)
{
    this->xMin = xMin;
    this->xMax = xMax;
    this->yMin = yMin;
    this->yMax = yMax;
}

bool StreetDbAnalyzer::IsDbOpened(void) const
{
    return dbOpened;
}

void StreetDbAnalyzer::ReadDatasetType(std::wstring datasetName) const
{
    fgdbError result;
    std::wstring datasetType;
    std::vector<std::string> childDatasetDefinitions;

    std::wcout << L"Reading primary dataset type..." << std::endl;
    if ((result = geodatabase.GetChildDatasetDefinitions(datasetName, datasetType, childDatasetDefinitions)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the barricade dataset definition info!", result);
        return;
    }

    std::wcout << L"The " << datasetName << " dataset is of type '" << datasetType << "'." << std::endl;
    for (unsigned int i = 0; i < childDatasetDefinitions.size(); i++)
    {
        std::cout << "  " << childDatasetDefinitions[i] << std::endl;
    }
}

void StreetDbAnalyzer::Analyze(void) const
{
    // Types
    std::vector<std::wstring> datasetTypes;
    
    std::wcout << L"Reading dataset types..." << std::endl;
    fgdbError result;
    if ((result = geodatabase.GetDatasetTypes(datasetTypes)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the dataset types!", result);
        return;
    }

    std::wcout << L"Read " << datasetTypes.size() << " dataset types." << std::endl;
    for (unsigned int i = 0; i < datasetTypes.size(); i++)
    {
        std::wcout << L"  " << datasetTypes[i] << std::endl;
    }

    // Relationship types
    std::vector<std::wstring> datasetRelationshipTypes;

    std::wcout << L"Reading dataset relationship types..." << std::endl;
    if ((result = geodatabase.GetDatasetRelationshipTypes(datasetTypes)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the dataset relationship types!", result);
        return;
    }

    std::wcout << L"Read " << datasetRelationshipTypes.size() << " dataset relationship types." << std::endl;
    for (unsigned int i = 0; i < datasetRelationshipTypes.size(); i++)
    {
        std::wcout << L"  " << datasetRelationshipTypes[i] << std::endl;
    }

    // Domains
    std::vector<std::wstring> datasetDomains;

    std::wcout << L"Reading dataset domains..." << std::endl;
    if ((result = geodatabase.GetDomains(datasetTypes)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the dataset domains!", result);
        return;
    }

    std::wcout << L"Read " << datasetDomains.size() << " dataset domains." << std::endl;
    for (unsigned int i = 0; i < datasetDomains.size(); i++)
    {
        std::wcout << L"  " << datasetDomains[i] << std::endl;
    }
    
    std::vector<std::wstring> childDatasets;

    std::wcout << L"Reading child datasets..." << std::endl;
    if ((result = geodatabase.GetChildDatasets(L"\\", L"", childDatasets)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the child datasets!", result);
        return;
    }

    std::wcout << L"Read " << childDatasets.size() << " child datasets." << std::endl;
    for (unsigned int i = 0; i < childDatasets.size(); i++)
    {
        std::wcout << L"  " << childDatasets[i] << std::endl;
    }

    ReadDatasetType(barricade.name);
    ReadDatasetType(stops.name);
    ReadDatasetType(streets.name);
    ReadDatasetType(emitter.name);
    ReadDatasetType(freewayModifier.name);
}

bool StreetDbAnalyzer::LoadTables()
{
    fgdbError result;
    std::wcout << "Reading in the barricade table..." << std::endl;
    if ((result = geodatabase.OpenTable(barricade.name, barricade.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table!", result);
        return false;
    }

    std::wcout << "Reading in the stops table..." << std::endl;
    if ((result = geodatabase.OpenTable(stops.name, stops.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table!", result);
        return false;
    }

    std::wcout << "Reading in the streets table..." << std::endl;
    if ((result = geodatabase.OpenTable(streets.name, streets.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table!", result);
        return false;
    }

    std::wcout << "Reading in the emitter table..." << std::endl;
    if ((result = geodatabase.OpenTable(emitter.name, emitter.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table!", result);
        return false;
    }

    std::wcout << "Reading in the freeway modifier table..." << std::endl;
    if ((result = geodatabase.OpenTable(freewayModifier.name, freewayModifier.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table!", result);
        return false;
    }

    std::wcout << "Tables opened." << std::endl;
    return true;
}

void StreetDbAnalyzer::AnalyzeTable(std::string prefix, const FileGDBAPI::Table& table) const
{
    std::string tableDefinition;

    fgdbError result;
    if ((result = table.GetDefinition(tableDefinition)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table definition!", result);
        return;
    }

    std::stringstream tableName;
    tableName << prefix << "_table.xml";
    std::ofstream defFile(tableName.str());
    if (!defFile)
    {
        std::cout << "Could not open the file to write the table definition to!" << std::endl;
        return;
    }

    defFile << tableDefinition.c_str();
    defFile.close();
    std::cout << "Table definition has been written to a readable file." << std::endl;

    std::string documentation;
    if ((result = table.GetDocumentation(documentation)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table documentation!", result);
        return;
    }

    std::stringstream docName;
    docName << prefix << "_documentation.xml";
    std::ofstream docFile(docName.str());
    if (!docFile)
    {
        std::cout << "Could not open the file to write documentation to!" << std::endl;
        return;
    }

    docFile << documentation.c_str();
    docFile.close();
    std::cout << "Documentation has been written to a readable-file." << std::endl;
}

void StreetDbAnalyzer::AnalyzeTables() const
{
    AnalyzeTable("barricade", barricade.table);
    AnalyzeTable("stops", stops.table);
    AnalyzeTable("streets", streets.table);
    AnalyzeTable("emitter", emitter.table);
    AnalyzeTable("freeway", freewayModifier.table);
}

bool LoadAllTableRows(Dataset& dataset)
{
    std::wcout << dataset.name << std::endl;

    fgdbError result;

    FileGDBAPI::Envelope extent;
    if ((result = dataset.table.GetExtent(extent)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table extent!", result);
        return false;
    }

    if ((result = dataset.table.GetRowCount(dataset.rowCount)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table row count!", result);
        return false;
    }

    std::cout << "Rows: " << dataset.rowCount << std::endl;

    std::cout << "Searching ALL rows in the database..." << std::endl;
    if ((result = dataset.table.Search(L"*", L"", extent, true, dataset.rows)) != S_OK)
    {
        ErrorLogger::LogError(L"Error searching for all rows!", result);
        return false;
    }

    return true;
}

void StreetDbAnalyzer::LoadAllRows()
{
    if (!LoadAllTableRows(barricade) || 
        !LoadAllTableRows(stops) ||
        !LoadAllTableRows(streets) ||
        !LoadAllTableRows(emitter) ||
        !LoadAllTableRows(freewayModifier))
    {
        // Exit early if loading anything fails.
        return;
    }
}

void StreetDbAnalyzer::ProcessBarricadeRows()
{
    std::ofstream dataFile("barricade.bin", std::ios::out | std::ios::binary);
    if (!dataFile)
    {
        std::cout << "Could not open the barricade file to write the data to!" << std::endl;
        return;
    }

    std::cout << "Barricade rows..." << std::endl;
    FileGDBAPI::Row row;
    while (barricade.rows.Next(row) == S_OK)
    {
        // Get the barricade positions.
        fgdbError result;
        FileGDBAPI::ShapeBuffer shapeBuffer;
        if ((result = row.GetGeometry(shapeBuffer)) != S_OK)
        {
            ErrorLogger::LogError(L"Error loading the shape buffer!", result);
            return; // Honestly this could be cleaner but as a single-use script more or less, I don't need to properly cleanup on failures.
        }
    }

    dataFile.close();
    barricade.rows.Close();
}

void StreetDbAnalyzer::ProcessStopsRows()
{
    std::ofstream dataFile("stops.bin", std::ios::out | std::ios::binary);
    if (!dataFile)
    {
        std::cout << "Could not open the stops file to write the data to!" << std::endl;
        return;
    }

    std::cout << "Stops rows..." << std::endl;
    FileGDBAPI::Row row;
    while (stops.rows.Next(row) == S_OK)
    {
        // TODO process
    }

    dataFile.close();
    stops.rows.Close();
}

void StreetDbAnalyzer::ProcessStreetsRows()
{
    std::ofstream dataFile("streets.bin", std::ios::out | std::ios::binary);
    if (!dataFile)
    {
        std::cout << "Could not open the streets file to write the data to!" << std::endl;
        return;
    }

    std::cout << "Streets rows..." << std::endl;
    FileGDBAPI::Row row;
    while (streets.rows.Next(row) == S_OK)
    {
        // TODO process
    }

    dataFile.close();
    streets.rows.Close();
}

void StreetDbAnalyzer::ProcessEmitterRows()
{
    std::ofstream dataFile("emitter.bin", std::ios::out | std::ios::binary);
    if (!dataFile)
    {
        std::cout << "Could not open the emitter file to write the data to!" << std::endl;
        return;
    }

    std::cout << "Emitter rows..." << std::endl;
    FileGDBAPI::Row row;
    while (emitter.rows.Next(row) == S_OK)
    {
        // TODO process
    }

    dataFile.close();
    emitter.rows.Close();
}

void StreetDbAnalyzer::ProcessFreewayModifierRows()
{
    std::ofstream dataFile("freewayModifier.bin", std::ios::out | std::ios::binary);
    if (!dataFile)
    {
        std::cout << "Could not open the freeway modifier file to write the data to!" << std::endl;
        return;
    }

    std::cout << "Freeway Modifier rows..." << std::endl;
    FileGDBAPI::Row row;
    while (freewayModifier.rows.Next(row) == S_OK)
    {
        // TODO process
    }

    dataFile.close();
    freewayModifier.rows.Close();
}

void StreetDbAnalyzer::UnloadTables()
{
    // Open the primary table
    fgdbError result;
    std::wcout << "Closing barricade table..." << std::endl;
    if ((result = geodatabase.CloseTable(barricade.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Closing stops table..." << std::endl;
    if ((result = geodatabase.CloseTable(stops.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Closing streets table..." << std::endl;
    if ((result = geodatabase.CloseTable(streets.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Closing emitter table..." << std::endl;
    if ((result = geodatabase.CloseTable(emitter.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Closing freeway modifier table..." << std::endl;
    if ((result = geodatabase.CloseTable(freewayModifier.table)) != S_OK)
    {
        ErrorLogger::LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Tables closed." << std::endl;
}

StreetDbAnalyzer::~StreetDbAnalyzer(void)
{
}

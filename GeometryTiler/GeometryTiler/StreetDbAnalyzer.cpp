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

void StreetDbAnalyzer::SetRescaleFactors(double xMin, double xMax, double yMin, double yMax)
{
    this->xMin = xMin;
    this->xMax = xMax;
    this->yMin = yMin;
    this->yMax = yMax;
}

void StreetDbAnalyzer::RescaleToFractions(double* x, double* y)
{
    *x = (*x - xMin) / (xMax - xMin);
    *y = (*y - yMin) / (yMax - yMin);
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

bool StreetDbAnalyzer::LoadAllTableRows(Dataset& dataset)
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

void StreetDbAnalyzer::SavePointRow(FileGDBAPI::Row& row, std::ofstream& dataFile)
{
    // Get the barricade positions.
    fgdbError result;
    FileGDBAPI::ShapeBuffer shapeBuffer;
    if ((result = row.GetGeometry(shapeBuffer)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the shape buffer!", result);
        return; // Honestly this could be cleaner but as a single-use script more or less, I don't need full error handling.
    }

    long type = shapeBuffer.shapeBuffer[0];
    double x = *reinterpret_cast<double*>(&shapeBuffer.shapeBuffer[sizeof(long)]);
    double y = *reinterpret_cast<double*>(&shapeBuffer.shapeBuffer[sizeof(long) + sizeof(double)]);
    RescaleToFractions(&x, &y);
    if (x < 0 || x > 1 || y < 0 || y > 1)
    {
        std::cout << "Not in area: " << x << ", " << y << ", ";
    }

    dataFile.write((char*)&x, sizeof(double));
    dataFile.write((char*)&y, sizeof(double));
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
    dataFile.write((char*)&barricade.rowCount, sizeof(int));
    while (barricade.rows.Next(row) == S_OK)
    {
        SavePointRow(row, dataFile);
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
    dataFile.write((char*)&stops.rowCount, sizeof(int));
    while (stops.rows.Next(row) == S_OK)
    {
        SavePointRow(row, dataFile);
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

    std::vector<SLineStrip> lineStrips;

    std::cout << "Streets rows..." << std::endl;
    FileGDBAPI::Row row;
    while (streets.rows.Next(row) == S_OK)
    {
        fgdbError result;

        std::wstring streetType;
        row.GetString(L"ST_TYPE", streetType);
        // if ((result = ) != S_OK)
        // {
        //     ErrorLogger::LogError(L"Error loading the street type!", result);
        // }

        FileGDBAPI::ShapeBuffer shapeBuffer;
        if ((result = row.GetGeometry(shapeBuffer)) != S_OK)
        {
            ErrorLogger::LogError(L"Error loading the shape buffer!", result);
            return; // Honestly this could be cleaner but as a single-use script more or less, I don't need full error handling.
        }

        // Get the street strips
        // At this point we know (from our saved table.xml file) that this is a polyline
        long type = shapeBuffer.shapeBuffer[0];
        
        // See the extended_shape_buffer_format.PDF file detailing the polyline shape
        long numParts = *reinterpret_cast<long*>(&shapeBuffer.shapeBuffer[sizeof(long) + 4*sizeof(double)]);
        long numPoints = *reinterpret_cast<long*>(&shapeBuffer.shapeBuffer[sizeof(long) + 4*sizeof(double) + sizeof(long)]);

        // There then follows an array 'numParts' long with 'long' indices for parts.
        // Each part represents the index of the *next* part. For example, if there are two parts with 
        // data '0' and '2', there is a line from points 0-1 and then a line from 2-onwards.
        // We don't need that confusion so when we save the data we split parts into their own line segment grouping.
        byte* partsArray = &shapeBuffer.shapeBuffer[sizeof(long) + 4*sizeof(double) + sizeof(long) + sizeof(long)];
        byte* pointsArray = partsArray + sizeof(long)*numParts;

        SLineStrip strip;
        strip.streetType = streetType;

        int currentPartIndex = 1;
        for (int i = 0; i < numPoints; i++)
        {
            SPoint* point = reinterpret_cast<SPoint*>(pointsArray + sizeof(SPoint)*i);

            // If we aren't on the last part and our current index is a new part...
            if (currentPartIndex != numParts && *reinterpret_cast<long*>((partsArray + sizeof(long)*currentPartIndex)) == i)
            {
                // ...save the current strip and start a new one.
                lineStrips.push_back(strip);
                strip.points.clear();
            }

            strip.points.push_back(*point);
        }

        // Add in the last strip.
        lineStrips.push_back(strip);
    }

    // Now combine the data and then write it out to our file.
    std::map<std::wstring, std::vector<std::vector<SPoint>>> roadLines;
    for (int i = 0; i < lineStrips.size(); i++)
    {
        // Remap out weird and invalid street types to valid types.
        if (lineStrips[i].streetType.compare(L"VIS") == 0 ||
            lineStrips[i].streetType.compare(L"VIEW") == 0 ||
            lineStrips[i].streetType.compare(L"SQ") == 0 ||
            lineStrips[i].streetType.compare(L"TER") == 0 ||
            lineStrips[i].streetType.compare(L"CT") == 0 ||
            lineStrips[i].streetType.compare(L"CIR") == 0 ||
            lineStrips[i].streetType.compare(L"CTF") == 0 ||
            lineStrips[i].streetType.compare(L"CRST") == 0 || 
            lineStrips[i].streetType.compare(L"CRES") == 0 ||
            lineStrips[i].streetType.compare(L"TRL") == 0 ||
            lineStrips[i].streetType.compare(L"RNCH") == 0 ||
            lineStrips[i].streetType.compare(L"TRL") == 0 ||
            lineStrips[i].streetType.compare(L"ALY") == 0 ||
            lineStrips[i].streetType.compare(L"KY") == 0 ||
            lineStrips[i].streetType.compare(L"LOOP") == 0 ||
            lineStrips[i].streetType.compare(L"11TH") == 0)
        {
            lineStrips[i].streetType = L"AVE";
        }
        else if (lineStrips[i].streetType.compare(L"STR") == 0 ||
                 lineStrips[i].streetType.compare(L"PARK") == 0 ||
                 lineStrips[i].streetType.compare(L"PT") == 0)
        {
            lineStrips[i].streetType = L"ST";
        }
        else if (lineStrips[i].streetType.compare(L"RD") == 0 ||
                 lineStrips[i].streetType.compare(L"LN") == 0 ||
                 lineStrips[i].streetType.compare(L" LN") == 0 ||
                 lineStrips[i].streetType.compare(L"WALK") == 0)
        {
            lineStrips[i].streetType = L"PL";
        }
        else if (lineStrips[i].streetType.compare(L"BLVD") == 0 ||
                 lineStrips[i].streetType.compare(L"DR") == 0 )
        {
            lineStrips[i].streetType = L"WAY";
        }
        else if (lineStrips[i].streetType.compare(L"HWY") == 0 ||
                 lineStrips[i].streetType.compare(L"") == 0 ||
                 lineStrips[i].streetType.compare(L" ") == 0 ||
                 lineStrips[i].streetType.compare(L"PKWY") == 0 || 
                 lineStrips[i].streetType.compare(L"BRG") == 0)
        {
            lineStrips[i].streetType = L"FWY";
        }


        if (roadLines.find(lineStrips[i].streetType) == roadLines.end())
        {
            roadLines[lineStrips[i].streetType] = std::vector<std::vector<SPoint>>();
        }

        roadLines[lineStrips[i].streetType].push_back(lineStrips[i].points);
    }

    for (auto iter = roadLines.begin(); iter != roadLines.end(); iter++)
    {
        std::wcout << L"  " << iter->first << L":" << iter->second.size() << std::endl;
    }

    // We have five types -- AVE, ST, PL, WAY, and FWY -- but might as well place them out here.
    int types = roadLines.size();
    dataFile.write((char*)&types, sizeof(int));
    
    std::vector<std::wstring> mainTypes;
    mainTypes.push_back(L"AVE");
    mainTypes.push_back(L"ST");
    mainTypes.push_back(L"PL");
    mainTypes.push_back(L"WAY");
    mainTypes.push_back(L"FWY");

    for (int i = 0; i < mainTypes.size(); i++)
    {
        int stripCount = roadLines[mainTypes[i]].size();
        dataFile.write((char*)&stripCount, sizeof(int));
        for (int j = 0; j < stripCount; j++)
        {
            int lineCount = roadLines[mainTypes[i]][j].size();
            dataFile.write((char*)&lineCount, sizeof(int));
            for (int k = 0; k < lineCount; k++)
            {
                SPoint point = roadLines[mainTypes[i]][j][k];
                RescaleToFractions(&point.x, &point.y);
                dataFile.write((char*)&point.x, sizeof(double));
                dataFile.write((char*)&point.y, sizeof(double));
            }
        }
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
    dataFile.write((char*)&emitter.rowCount, sizeof(int));
    while (emitter.rows.Next(row) == S_OK)
    {
        SavePointRow(row, dataFile);
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
    dataFile.write((char*)&freewayModifier.rowCount, sizeof(int));
    while (freewayModifier.rows.Next(row) == S_OK)
    {
        SavePointRow(row, dataFile);
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>
#include <limits>
#include "GeodatabaseAnalyzer.h"

GeodatabaseAnalyzer::GeodatabaseAnalyzer(std::wstring databasePath, std::wstring primaryDataset)
    : dbOpened(false), primaryDataset(primaryDataset)
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

bool GeodatabaseAnalyzer::IsDbOpened(void) const
{
    return dbOpened;
}

void GeodatabaseAnalyzer::Analyze(void) const
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

    std::wstring datasetType;
    std::vector<std::string> childDatasetDefinitions;

    std::wcout << L"Reading primary dataset type..." << std::endl;
    if ((result = geodatabase.GetChildDatasetDefinitions(primaryDataset, datasetType, childDatasetDefinitions)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the primary dataset definition info!", result);
        return;
    }

    std::wcout << L"The primary dataset is of type '" << datasetType << "'." << std::endl;
    for (unsigned int i = 0; i < childDatasetDefinitions.size(); i++)
    {
        std::cout << "  " << childDatasetDefinitions[i] << std::endl;
    }
}

bool GeodatabaseAnalyzer::LoadTable()
{
    fgdbError result;
    std::wcout << "Reading in the primary table..." << std::endl;
    if ((result = geodatabase.OpenTable(L"\\CONTOUR005_LINE", primaryTable)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table!", result);
        return false;
    }

    std::wcout << "Primary table opened." << std::endl;
    return true;
}

void GeodatabaseAnalyzer::AnalyzeTable(void) const
{
    std::string tableDefinition;

    fgdbError result;
    if ((result = primaryTable.GetDefinition(tableDefinition)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table definition!", result);
        return;
    }

    std::ofstream defFile("table.xml");
    if (!defFile)
    {
        std::cout << "Could not open the file to write the table definition to!" << std::endl;
        return;
    }

    defFile << tableDefinition.c_str();
    defFile.close();
    std::cout << "Table definition has been written to a readable file." << std::endl;

    std::string documentation;
    if ((result = primaryTable.GetDocumentation(documentation)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table documentation!", result);
        return;
    }

    std::ofstream docFile("documentation.xml");
    if (!docFile)
    {
        std::cout << "Could not open the file to write documentation to!" << std::endl;
        return;
    }

    docFile << documentation.c_str();
    docFile.close();
    std::cout << "Documentation has been written to a readable-file." << std::endl;
}

void GeodatabaseAnalyzer::LoadAllRows()
{
    fgdbError result;

    FileGDBAPI::Envelope extent;
    if ((result = primaryTable.GetExtent(extent)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table extent!", result);
        return;
    }

    std::cout << "Extent: [" << extent.xMin << ", " << extent.yMin << ", " << extent.zMin << "] to [" 
        << extent.xMax << ", " << extent.yMax << ", " << extent.zMax << "]" << std::endl;

    if ((result = primaryTable.GetRowCount(rowCount)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the table row count!", result);
        return;
    }

    std::cout << "Rows: " << rowCount << std::endl;

    std::cout << "Searching ALL rows in the database..." << std::endl;
    if ((result = primaryTable.Search(L"*", L"", extent, true, allRows)) != S_OK)
    {
        ErrorLogger::LogError(L"Error searching for all rows!", result);
        return;
    }
}

void GeodatabaseAnalyzer::CheckLimits(Point point)
{
    if (point.x < minX)
    {
        minX = point.x;
    }

    if (point.x > maxX)
    {
        maxX = point.x;
    }
    
    if (point.y < minY)
    {
        minY = point.y;
    }

    if (point.y > maxY)
    {
        maxY = point.y;
    }
}

long GeodatabaseAnalyzer::ProcessRow(FileGDBAPI::Row& row, std::vector<LineStrip>& lineStrips)
{
    fgdbError result;
    FileGDBAPI::ShapeBuffer shapeBuffer;
    if ((result = row.GetGeometry(shapeBuffer)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the shape buffer!", result);
        return 0;
    }

    double elevation; // Can be negative!
    if ((result = row.GetDouble(L"ELEVATION", elevation)) != S_OK)
    {
        ErrorLogger::LogError(L"Error loading the shape buffer!", result);
        return 0;
    }

    // At this point we know (from our saved table.xml file) that this is a polyline, and (from the above), we know the elevation.
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

    // std::cout << type << " " << numParts << " " << numPoints << " ";
    for (int i = 0; i < numParts; i++)
    {
       //  std::cout << (long)(*(partsArray + sizeof(long)*i)) << " ";
    }
    // std::cout << std::endl;
    if (numPoints == 0)
    {
        std::cout << "hit!" << std::endl;
    }

    LineStrip strip;
    strip.elevation = elevation;
    if (elevation < minElevation)
    {
        minElevation = elevation;
    }
    if (elevation > maxElevation)
    {
        maxElevation = elevation;
    }

    int currentPartIndex = 1;
    for (int i = 0; i < numPoints; i++)
    {
        Point* point = reinterpret_cast<Point*>(pointsArray + sizeof(Point)*i);

        // If we aren't on the last part and our current index is a new part...
        if (currentPartIndex != numParts && *reinterpret_cast<long*>((partsArray + sizeof(long)*currentPartIndex)) == i)
        {
            // ...save the current strip and start a new one.
            lineStrips.push_back(strip);
            strip.points.clear();
        }

        CheckLimits(*point);
        strip.points.push_back(*point);
    }

    // Add in the last strip.
    lineStrips.push_back(strip);
    return numPoints;
}

void GeodatabaseAnalyzer::ProcessAllRows()
{
    std::cout << "Processing returned rows..." << std::endl;
    
    minX = std::numeric_limits<double>::max();
    maxX = std::numeric_limits<double>::min();
    minY = std::numeric_limits<double>::max();
    maxY = std::numeric_limits<double>::min();
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();

    std::vector<LineStrip> lineStrips;
    std::ofstream dataFile("data.bin", std::ios::out | std::ios::binary);
    if (!dataFile)
    {
        std::cout << "Could not open the file to write the data to!" << std::endl;
        return;
    }

    // We seek and overwrite this data after performing all the processing.
    int lineStripCount = 0;
    dataFile.write((char*)&lineStripCount, sizeof(int));
    dataFile.write((char*)&minX, sizeof(double));
    dataFile.write((char*)&maxX, sizeof(double));
    dataFile.write((char*)&minY, sizeof(double));
    dataFile.write((char*)&maxY, sizeof(double));
    dataFile.write((char*)&minElevation, sizeof(double));
    dataFile.write((char*)&maxElevation, sizeof(double));

    int points = 0;
    FileGDBAPI::Row row;
    returnedRows = 0;
    while (allRows.Next(row) == S_OK)
    {
        ++returnedRows;
        if (returnedRows % 500 == 0)
        {
            std::cout << "  " << returnedRows << std::endl;
        }

        points = ProcessRow(row, lineStrips);

        for (int i = 0; i < lineStrips.size(); i++)
        {
            dataFile.write((char*)&(lineStrips[i].elevation), sizeof(double));
            int pointCount = (int)lineStrips[i].points.size();

            dataFile.write((char*)&pointCount, sizeof(int));
            dataFile.write((char*)&(lineStrips[i].points[0]), sizeof(Point)*pointCount);

            ++lineStripCount;
        }

        lineStrips.clear();
    }

    // Overwrite the number of line strips.
    dataFile.seekp(0, std::ios::beg);
    dataFile.write((char*)&lineStripCount, sizeof(int));
    dataFile.write((char*)&minX, sizeof(double));
    dataFile.write((char*)&maxX, sizeof(double));
    dataFile.write((char*)&minY, sizeof(double));
    dataFile.write((char*)&maxY, sizeof(double));
    dataFile.write((char*)&minElevation, sizeof(double));
    dataFile.write((char*)&maxElevation, sizeof(double));
    dataFile.close();

    std::cout << "Counted " << returnedRows << " returned rows (versus " << rowCount << " total rows)" << std::endl;
    if (returnedRows != rowCount)
    {
        std::cout << "Warning: Didn't read all the data from the database!" << std::endl;
        return;
    }

    std::cout << "Total of " << points << " total geometry points." << std::endl;
    std::cout << "Limits computed to be [" << minX << ", " << maxX << "; " << minY << ", " << maxY << "; " << minElevation << ", " << maxElevation << "]." << std::endl;
    allRows.Close();
}

void GeodatabaseAnalyzer::UnloadTable()
{
    // Open the primary table
    fgdbError result;
    std::wcout << "Closing the primary table..." << std::endl;
    if ((result = geodatabase.CloseTable(primaryTable)) != S_OK)
    {
        ErrorLogger::LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Primary table closed." << std::endl;
}

GeodatabaseAnalyzer::~GeodatabaseAnalyzer(void)
{
    fgdbError result;
    std::wcout << "Closing DB..." << std::endl;
    if ((result = FileGDBAPI::CloseGeodatabase(geodatabase)) != S_OK)
    {
        ErrorLogger::LogError(L"Could not close the Geodatabase!", result);

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        return;
    }

    std::wcout << "DB closed." << std::endl;
}

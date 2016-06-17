// GeometryTiler.cpp : Defines the entry point for the console application.
//

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <FileGDBAPI.h>

#ifndef _DEBUG
    #pragma comment(lib, "lib/FileGDBAPI.lib")
#else
    #pragma comment(lib, "lib/FileGDBAPID.lib")
#endif

// Nicely logs the provided error message and error code.
void LogError(std::wstring errorMessage, fgdbError errorCode)
{
    std::wcout << errorMessage << std::endl;
    
    std::wstring errorText;
    FileGDBAPI::ErrorInfo::GetErrorDescription(errorCode, errorText);
    std::wcout << "Error: " << errorText << "[" << errorCode << "]" << std::endl;
}

// Processes an individual row.
long totalPoints = 0;
void ProcessRow(FileGDBAPI::Row& row)
{
    // TODO this is a good point to move into a class. 
    fgdbError result;
    FileGDBAPI::ShapeBuffer shapeBuffer;
    if ((result = row.GetGeometry(shapeBuffer)) != S_OK)
    {
        LogError(L"Error loading the shape buffer!", result);
        return;
    }

    double elevation; // Can be negative!
    if ((result = row.GetDouble(L"ELEVATION", elevation)) != S_OK)
    {
        LogError(L"Error loading the shape buffer!", result);
        return;
    }


    // At this point we know (from our saved table.xml file) that this is a polyline, and (from the above), we know the elevation.
    long type = shapeBuffer.shapeBuffer[0];
    
    // See the extended_shape_buffer_format.PDF file detailing the polyline shape
    long numParts = shapeBuffer.shapeBuffer[sizeof(long) + 4*sizeof(double)];
    long numPoints = shapeBuffer.shapeBuffer[sizeof(long) + 4*sizeof(double) + sizeof(long)];
    
    // std::cout << type << " " << numParts << " " << numPoints << std::endl;
    /*if ((result = shapeBuffer.GetNumPoints(numPoints)) != S_OK)
    {
        LogError(L"Error loading the number of points in the shape!", result);
        return;
    }*/
    
    totalPoints += numPoints;
}

// Processes the provided Table.
void ProcessTable(FileGDBAPI::Table& table)
{
    std::string tableDefinition;

    fgdbError result;
    if ((result = table.GetDefinition(tableDefinition)) != S_OK)
    {
        LogError(L"Error loading the table definition!", result);
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
    if ((result = table.GetDocumentation(documentation)) != S_OK)
    {
        LogError(L"Error loading the table documentation!", result);
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

    FileGDBAPI::Envelope extent;
    if ((result = table.GetExtent(extent)) != S_OK)
    {
        LogError(L"Error loading the table extent!", result);
        return;
    }

    std::cout << "Extent: [" << extent.xMin << ", " << extent.yMin << ", " << extent.zMin << "] to [" 
        << extent.xMax << ", " << extent.yMax << ", " << extent.zMax << "]" << std::endl;

    int rowCount;
    if ((result = table.GetRowCount(rowCount)) != S_OK)
    {
        LogError(L"Error loading the table row count!", result);
        return;
    }

    std::cout << "Rows: " << rowCount << std::endl;

    FileGDBAPI::EnumRows allRows;
    std::cout << "Searching ALL rows in the database..." << std::endl;
    if ((result = table.Search(L"*", L"", extent, true, allRows)) != S_OK)
    {
        LogError(L"Error searching for all rows!", result);
        return;
    }

    std::cout << "Counting returned rows..." << std::endl;
    FileGDBAPI::Row row;
    int foundRowCount = 0;
    while (allRows.Next(row) == S_OK)
    {
        ++foundRowCount;
        if (foundRowCount % 1000 == 0)
        {
            std::cout << "  " << foundRowCount << std::endl;
        }

        ProcessRow(row);
    }

    allRows.Close();
    std::cout << "Counted " << foundRowCount << " returned rows (versus " << rowCount << " total rows)" << std::endl;
    if (foundRowCount != rowCount)
    {
        std::cout << "Warning: Didn't read all the data from the database!" << std::endl;
        return;
    }

    std::cout << "Total of " << totalPoints << " total geometry points." << std::endl;
}

// Processes the provided Geodatabase.
void ProcessGeodatabase(FileGDBAPI::Geodatabase& geodatabase)
{
    // Types
    std::vector<std::wstring> datasetTypes;
    
    std::wcout << L"Reading dataset types..." << std::endl;
    fgdbError result;
    if ((result = geodatabase.GetDatasetTypes(datasetTypes)) != S_OK)
    {
        LogError(L"Error loading the dataset types!", result);
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
        LogError(L"Error loading the dataset relationship types!", result);
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
        LogError(L"Error loading the dataset domains!", result);
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
        LogError(L"Error loading the child datasets!", result);
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
    if ((result = geodatabase.GetChildDatasetDefinitions(L"\\CONTOUR005_LINE", datasetType, childDatasetDefinitions)) != S_OK)
    {
        LogError(L"Error loading the primary dataset definition info!", result);
        return;
    }

    std::wcout << L"The primary dataset is of type '" << datasetType << "'." << std::endl;
    for (unsigned int i = 0; i < childDatasetDefinitions.size(); i++)
    {
        std::cout << "  " << childDatasetDefinitions[i] << std::endl;
    }

    // Open the primary table
    FileGDBAPI::Table table;
    std::wcout << "Reading in the primary table..." << std::endl;
    if ((result = geodatabase.OpenTable(L"\\CONTOUR005_LINE", table)) != S_OK)
    {
        LogError(L"Error loading the table!", result);
        return;
    }

    std::wcout << "Primary table opened." << std::endl;

    ProcessTable(table);

    std::wcout << "Closing the primary table..." << std::endl;
    if ((result = geodatabase.CloseTable(table)) != S_OK)
    {
        LogError(L"Error closing the table!", result);
        return;
    }

    std::wcout << "Primary table closed." << std::endl;
}

int main(int argc, const char* argv[])
{
    // Honestly this should be a command-line argument but this is just a basic converter tool.
    std::wstring databasePath =
        L"C:\\Users\\Gustave\\Desktop\\KingCounty_GDB_topo_contour005_complete - Copy\\KingCounty_GDB_topo_contour005_complete.gdb";

    FileGDBAPI::Geodatabase geodatabase;

    fgdbError result;
    std::wcout << "Opening the db..." << std::endl;
    if ((result = FileGDBAPI::OpenGeodatabase(databasePath, geodatabase)) != S_OK)
    {
        LogError(L"Could not open the Geodatabase!", result);

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        return -1;
    }

    std::wcout << "DB opened." << std::endl;

    ProcessGeodatabase(geodatabase);

    std::wcout << "Closing DB..." << std::endl;
    if ((result = FileGDBAPI::CloseGeodatabase(geodatabase)) != S_OK)
    {
        LogError(L"Could not close the Geodatabase!", result);

        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        return -1;
    }

    std::wcout << "DB closed." << std::endl;
    std::system("pause");
    return 0;
}


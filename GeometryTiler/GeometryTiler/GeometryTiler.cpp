#include <string>
#include <chrono>
#include <ratio>
#include <thread>

#include <FileGDBAPI.h>
#include "GeodatabaseAnalyzer.h"
#include "StreetDbAnalyzer.h"

#ifndef _DEBUG
    #pragma comment(lib, "lib/FileGDBAPI.lib")
#else
    #pragma comment(lib, "lib/FileGDBAPID.lib")
#endif

// Runs the GeometryTiler application.
// This (misnamed) application converts the GDB pointline data into a binary series of lines (with elevation data).
// This data format is easier to process within the actual geometry tiler for heightmap generation.
int main(int argc, const char* argv[])
{
    // Honestly ese should be command-line arguments but this is just a basic converter tool.
    std::wstring databasePath =
        L"C:\\Users\\Gustave\\Desktop\\KingCounty_GDB_topo_contour005_complete - Copy\\KingCounty_GDB_topo_contour005_complete.gdb";
    std::wstring primaryDataset =
        L"\\CONTOUR005_LINE";
    
    std::wstring transportationDbPath = 
        L"C:\\Users\\Gustave\\Desktop\\transportationGDB\\KingCounty_GDB_transportation.gdb";
    std::wstring barricadeDataset = 
        L"\\barricade_point";
    std::wstring stopsDataset =
        L"\\stops_offset_all_point";
    std::wstring streetsDataset =
        L"\\st_address_line";
    std::wstring emitterDataset =
        L"\\emitter_point";
    std::wstring freewayModifierDataset = 
        L"\\freeway_speeds_actual_point";
    
    auto start = std::chrono::high_resolution_clock::now();

    /*
    GeodatabaseAnalyzer* analyzer = new GeodatabaseAnalyzer(databasePath, primaryDataset);
    if (analyzer->IsDbOpened())
    {
        analyzer->Analyze();
        if (analyzer->LoadTable())
        {
            analyzer->AnalyzeTable();
            analyzer->LoadAllRows();
            analyzer->ProcessAllRows();
            analyzer->UnloadTable();
        }
    }

    delete analyzer;
    */

    StreetDbAnalyzer* streetAnalyzer = new StreetDbAnalyzer(transportationDbPath, 
        barricadeDataset, stopsDataset, streetsDataset, emitterDataset, freewayModifierDataset);
    streetAnalyzer->SetRescaleFactors(1214976.797, 1583489.0, 22489.21725, 360015.6455); // Grabbed from the original geometry db.

    if (streetAnalyzer->IsDbOpened())
    {
        streetAnalyzer->Analyze();
        if (streetAnalyzer->LoadTables())
        {
            streetAnalyzer->AnalyzeTables();
            streetAnalyzer->LoadAllRows();

            streetAnalyzer->ProcessBarricadeRows();
            streetAnalyzer->ProcessStopsRows();
            streetAnalyzer->ProcessStreetsRows();
            streetAnalyzer->ProcessEmitterRows();
            streetAnalyzer->ProcessFreewayModifierRows();

            streetAnalyzer->UnloadTables();
        }
    }

    delete streetAnalyzer;

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Total conversion process took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
        << " ms." << std::endl;

    std::system("pause");
    return 0;
}
#include <string>
#include <chrono>
#include <ratio>
#include <thread>

#include <FileGDBAPI.h>
#include "GeodatabaseAnalyzer.h"

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
        L"<\\CONTOUR005_LINE";
    
    // Min x, y, elevation = 0.
    // This is roughly an equivalent downscaling using the boundaries of king county + highest mountain + most negative elevation.
    double maxX = 100000;
    double maxY = 100000;
    double maxElevation = 3000;

    auto start = std::chrono::high_resolution_clock::now();

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

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Total conversion process took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
        << " ms." << std::endl;

    std::system("pause");
    return 0;
}
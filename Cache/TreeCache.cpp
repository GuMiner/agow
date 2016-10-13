#include "TreeCache.h"

TreeCache::TreeCache(const std::string& cacheFolder, const std::string& cacheType)
    : TerrainCache(cacheFolder, cacheType)
{
}

void TreeCache::SaveData(std::ofstream* outputStream, void* data)
{
    TreeCacheInputData* cacheData = (TreeCacheInputData*)data;

    // Output count of each vector followed by the data in the vector.
    WriteVectorVector(outputStream, cacheData->branches);
    WriteVectorVector(outputStream, cacheData->branchColors);
    WriteUintVector(outputStream, cacheData->branchThicknesses);
    WriteVectorVector(outputStream, cacheData->leaves);
    WriteVectorVector(outputStream, cacheData->leafColors);
}

void TreeCache::LoadData(std::ifstream* inputStream, void** data)
{
    // In this use case, we're assuming we already have a valid TreeCacheInputData object.
    TreeCacheData* cacheData = (TreeCacheData*)*data;

    ReadVectorVector(inputStream, &cacheData->branches);
    ReadVectorVector(inputStream, &cacheData->branchColors);
    ReadUintVector(inputStream, &cacheData->branchThicknesses);
    ReadVectorVector(inputStream, &cacheData->leaves);
    ReadVectorVector(inputStream, &cacheData->leafColors);
}

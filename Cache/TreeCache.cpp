#include "TreeCache.h"

TreeCache::TreeCache(const std::string& cacheFolder, const std::string& cacheType)
    : TerrainCache(cacheFolder, cacheType)
{
}

void TreeCache::SaveData(std::ofstream* outputStream, void* data)
{
    TreeCacheInputData* cacheData = (TreeCacheInputData*)data;

    // Output count of each vector followed by the data in the vector.
    outputStream->write((char*)&cacheData->hasEffect, sizeof(bool));
    WriteVectorVector(outputStream, cacheData->branches);
    WriteVectorVector(outputStream, cacheData->branchColors);
    WriteVectorVector(outputStream, cacheData->leaves);
    WriteVectorVector(outputStream, cacheData->leafColors);
}

void TreeCache::LoadData(std::ifstream* inputStream, void** data)
{
    // In this use case, we're assuming we already have a valid TreeCacheInputData object.
    TreeCacheData* cacheData = (TreeCacheData*)*data;

    inputStream->read((char*)&cacheData->hasEffect, sizeof(bool));
    ReadVectorVector(inputStream, &cacheData->branches);
    ReadVectorVector(inputStream, &cacheData->branchColors);
    ReadVectorVector(inputStream, &cacheData->leaves);
    ReadVectorVector(inputStream, &cacheData->leafColors);
}

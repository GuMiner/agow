#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Math\Vec.h"

// Defines how to communicate with the cache of data in the terrain cache.
class TerrainCache
{
    std::string cacheFolder;
    std::string cacheType;

    std::string GetCacheFolder(const vec::vec2i& subtilePos);
    std::string GetCacheFilePath(const vec::vec2i& subtilePos);

protected:
    void WriteVectorVector(std::ofstream* outputStream, std::vector<vec::vec3>* vector);
    void ReadVectorVector(std::ifstream* inputStream, std::vector<vec::vec3>* vector);

    virtual void SaveData(std::ofstream* outputStream, void* data) = 0;
    virtual void LoadData(std::ifstream* inputStream, void** data) = 0;

public:
    TerrainCache(const std::string& cacheFolder, const std::string& cacheType);
    bool IsInCache(const vec::vec2i& subtilePos);
    void SaveToCache(const vec::vec2i& subtilePos, void* data);
    void LoadFromCache(const vec::vec2i& subtilePos, void** data);
};
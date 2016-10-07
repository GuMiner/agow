#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <glm\vec2.hpp>

// Defines how to communicate with the cache of data in the terrain cache.
class TerrainCache
{
    std::string cacheFolder;
    std::string cacheType;

    std::string GetCacheFolder(const glm::ivec2& subtilePos);
    std::string GetCacheFilePath(const glm::ivec2& subtilePos);

protected:
    void WriteVectorVector(std::ofstream* outputStream, std::vector<glm::vec3>* vector);
    void ReadVectorVector(std::ifstream* inputStream, std::vector<glm::vec3>* vector);

    virtual void SaveData(std::ofstream* outputStream, void* data) = 0;
    virtual void LoadData(std::ifstream* inputStream, void** data) = 0;

public:
    TerrainCache(const std::string& cacheFolder, const std::string& cacheType);
    bool IsInCache(const glm::ivec2& subtilePos);
    void SaveToCache(const glm::ivec2& subtilePos, void* data);
    void LoadFromCache(const glm::ivec2& subtilePos, void** data);
};
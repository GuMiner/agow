#include <direct.h>
#include <io.h>
#include <sstream>
#include "Utils\Logger.h"
#include "TerrainCache.h"

TerrainCache::TerrainCache(const std::string& cacheFolder, const std::string& cacheType)
    : cacheFolder(cacheFolder), cacheType(cacheType)
{
}

std::string TerrainCache::GetCacheFilePath(const glm::ivec2& subtilePos)
{
    std::stringstream cachePath;
    cachePath << cacheFolder << "/" << subtilePos.x << "/" << subtilePos.y << "_" << cacheType << ".data";
    return cachePath.str();
}

std::string TerrainCache::GetCacheFolder(const glm::ivec2& subtilePos)
{
    std::stringstream cacheFolderPath;
    cacheFolderPath << cacheFolder << "/" << subtilePos.x;
    return cacheFolderPath.str();
}

bool TerrainCache::IsInCache(const glm::ivec2& subtilePos)
{
    std::string cacheFilePath = GetCacheFilePath(subtilePos);
    std::string cacheFileFolder = GetCacheFolder(subtilePos);
    if (_access(cacheFileFolder.c_str(), 0) == 0)
    {
        if (_access(cacheFilePath.c_str(), 0) == 0)
        {
            return true;
        }
    }

    return false;
}

void TerrainCache::WriteVectorVector(std::ofstream* outputStream, std::vector<glm::vec3>* vector)
{
    unsigned int size = vector == nullptr ? 0 : vector->size();
    outputStream->write((char*)&size, sizeof(unsigned int));
    for (unsigned int i = 0; i < size; i++)
    {
        outputStream->write((char*)&((*vector)[i]), sizeof(glm::vec3));
    }
}

void TerrainCache::ReadVectorVector(std::ifstream* inputStream, std::vector<glm::vec3>* vector)
{
    unsigned int size;
    inputStream->read((char*)&size, sizeof(unsigned int));
    for (unsigned int i = 0; i < size; i++)
    {
        glm::vec3 point;
        inputStream->read((char*)&point, sizeof(glm::vec3));
        vector->push_back(point);
    }
}

void TerrainCache::SaveToCache(const glm::ivec2& subtilePos, void* data)
{
    std::string cacheFileFolder = GetCacheFolder(subtilePos);
    if (_access(cacheFileFolder.c_str(), 0) != 0)
    {
        // The folder for this cache file doesn't exist, so create it.
        _mkdir(cacheFileFolder.c_str());
    }

    std::ofstream cacheFile(GetCacheFilePath(subtilePos), std::ios::out | std::ios::binary);
    SaveData(&cacheFile, data);
    cacheFile.close();
}

void TerrainCache::LoadFromCache(const glm::ivec2& subtilePos, void** data)
{
    std::string cacheFilePath = GetCacheFilePath(subtilePos);
    std::ifstream cacheFile(cacheFilePath, std::ios::in | std::ios::binary);
    if (!cacheFile)
    {
        Logger::LogError("Failed to load the specified cache file '", cacheFilePath, "'!");
        *data = nullptr;
    }
    else
    {
        LoadData(&cacheFile, data);
        cacheFile.close();
    }
}
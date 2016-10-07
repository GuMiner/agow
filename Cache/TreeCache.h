#pragma once
#include <vector>
#include <glm\vec3.hpp>
#include "TerrainCache.h"

struct TreeCacheData
{
    bool hasEffect;
    std::vector<glm::vec3> branches;
    std::vector<glm::vec3> branchColors;

    std::vector<glm::vec3> leaves;
    std::vector<glm::vec3> leafColors;
};

struct TreeCacheInputData
{
    bool hasEffect;
    std::vector<glm::vec3>* branches;
    std::vector<glm::vec3>* branchColors;

    std::vector<glm::vec3>* leaves;
    std::vector<glm::vec3>* leafColors;

    TreeCacheInputData(bool hasEffect, std::vector<glm::vec3>* branches, std::vector<glm::vec3>* branchColors,
        std::vector<glm::vec3>* leaves, std::vector<glm::vec3>* leafColors)
        : hasEffect(hasEffect), branches(branches), branchColors(branchColors), leaves(leaves), leafColors(leafColors)
    {
    }
};

class TreeCache : public TerrainCache
{
public:
    TreeCache(const std::string& cacheFolder, const std::string& cacheType);

    // Loads and saves tree cache data.
    virtual void SaveData(std::ofstream* outputStream, void* data) override;
    virtual void LoadData(std::ifstream* inputStream, void** data) override;
};
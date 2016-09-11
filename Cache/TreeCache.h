#pragma once
#include <vector>
#include "Math\Vec.h"
#include "TerrainCache.h"

struct TreeCacheData
{
    bool hasEffect;
    std::vector<vec::vec3> branches;
    std::vector<vec::vec3> branchColors;

    std::vector<vec::vec3> leaves;
    std::vector<vec::vec3> leafColors;
};

struct TreeCacheInputData
{
    bool hasEffect;
    std::vector<vec::vec3>* branches;
    std::vector<vec::vec3>* branchColors;

    std::vector<vec::vec3>* leaves;
    std::vector<vec::vec3>* leafColors;

    TreeCacheInputData(bool hasEffect, std::vector<vec::vec3>* branches, std::vector<vec::vec3>* branchColors,
        std::vector<vec::vec3>* leaves, std::vector<vec::vec3>* leafColors)
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
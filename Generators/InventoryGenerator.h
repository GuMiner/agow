#pragma once
#include <string>
#include <vector>

// Randomly generates inventory for NPCs.
class InventoryGenerator
{
    static std::vector<std::string> commonItems;
    static std::vector<std::string> rareItems;

    static bool itemsLoaded;
    static void LoadItems();
public:
    static std::string GetRandomItem();
};


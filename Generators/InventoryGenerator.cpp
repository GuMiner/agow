#include <glm\gtc\random.hpp>
#include "InventoryGenerator.h"

std::vector<std::string> InventoryGenerator::commonItems;
std::vector<std::string> InventoryGenerator::rareItems;
bool InventoryGenerator::itemsLoaded = false;

void InventoryGenerator::LoadItems()
{
    // TODO configurable.
    commonItems.push_back("paper");
    commonItems.push_back("pencil");
    commonItems.push_back("loose change");
    commonItems.push_back("cell phone");
    
    // TODO configurable.
    rareItems.push_back("gem");
    rareItems.push_back("ammunition");
    rareItems.push_back(".45 pistol");
    rareItems.push_back("paper");
    rareItems.push_back("satellite phone");
}

std::string InventoryGenerator::GetRandomItem()
{
    if (!itemsLoaded)
    {
        LoadItems();
        itemsLoaded = true;
    }

    return glm::linearRand(0.0f, 1.0f) < 0.90f ? 
        commonItems[glm::linearRand(0, (int)commonItems.size() - 1)] :
        rareItems[glm::linearRand(0, (int)rareItems.size() - 1)];
}
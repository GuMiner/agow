#include "Math\MathOps.h"
#include "Utils\StringUtils.h"
#include "BuildingGenerator.h"

DecisionTree<BuildingDecisionData> BuildingGenerator::buildingBuilder(BuildingGenerator::DeserializeBuildingRule);

BuildingGenerator::BuildingGenerator()
{
}

BuildingDecisionData BuildingGenerator::DeserializeBuildingRule(std::string line, bool* success)
{
    BuildingDecisionData data;
    data.modelName = line;

    *success = true;
    return data;
}

bool BuildingGenerator::LoadBuilder(std::string builderDecisionTreeFile)
{
    return buildingBuilder.LoadTreeFromFile(builderDecisionTreeFile);
}

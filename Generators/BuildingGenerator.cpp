#include <Bullet\btBulletDynamicsCommon.h>
#include <glm\gtc\random.hpp>
#include "Utils\Logger.h"
#include "Utils\StringUtils.h"
#include "BuildingGenerator.h"

DecisionTree<BuildingDecisionData> BuildingGenerator::lowDensityBuildingBuilder(BuildingGenerator::DeserializeBuildingRule);
DecisionTree<BuildingDecisionData> BuildingGenerator::highDensityBuildingBuilder(BuildingGenerator::DeserializeBuildingRule);

BuildingGenerator::BuildingGenerator(ModelManager* modelManager, Physics* physics)
    : modelManager(modelManager), physics(physics)
{
}

BuildingDecisionData BuildingGenerator::DeserializeBuildingRule(std::string line, bool* success)
{
    BuildingDecisionData data;
    std::vector<std::string> lines;
    StringUtils::Split(line, StringUtils::Space, true, lines);
    
    if (lines.size() != 6)
    {
        Logger::LogError("Expected 6 entries, found ", lines.size(), ".");
        *success = false;
        return data;
    }

    data.modelName = lines[0];
    if (!StringUtils::ParseFloatFromString(lines[1], data.zFactor) ||
        !StringUtils::ParseFloatFromString(lines[2], data.minScaleFactor) ||
        !StringUtils::ParseFloatFromString(lines[3], data.maxScaleFactor) ||
        !StringUtils::ParseIntFromString(lines[4], data.minLayers) ||
        !StringUtils::ParseIntFromString(lines[5], data.maxLayers))
    {
        Logger::LogError("Error deserializing numerical types from the data line, ", line, ".");
        *success = false;
        return data;
    }

    // TODO don't assume the data is valid, perform some additional checking.
    *success = true;
    return data;
}

DecisionTree<BuildingDecisionData>::Choice BuildingGenerator::RandomWalkEvaluator(BuildingDecisionData decisionData, bool isValidSegment, bool yesNodeNotNull, bool noNodeNotNull)
{
    // We're randomly walking so we don't need any of the decision or validation data.
    if (yesNodeNotNull && noNodeNotNull)
    {
        // Both aren't null. Randomly choose.
        if (glm::linearRand(0.0f, 1.0f) > 0.50f)
        {
            return DecisionTree<BuildingDecisionData>::Choice::YES_ITEM;
        }

        return DecisionTree<BuildingDecisionData>::Choice::NO_ITEM;
    }
    else if (yesNodeNotNull)
    {
        return DecisionTree<BuildingDecisionData>::Choice::YES_ITEM;
    }
    else if (noNodeNotNull)
    {
        return DecisionTree<BuildingDecisionData>::Choice::NO_ITEM;
    }

    return DecisionTree<BuildingDecisionData>::Choice::THIS_ITEM;
}

bool BuildingGenerator::LoadBuildingModels(ModelManager* modelManager)
{
    // Segments
    unsigned int modelId = modelManager->LoadModel("models/buildings/segments/circle");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/segments/square");
    if (modelId == 0)
    {
        return false;
    }

    // Tops
    modelId = modelManager->LoadModel("models/buildings/tops/dome");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/flat");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/pointy");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/pyramid");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/turret");
    if (modelId == 0)
    {
        return false;
    }

    return true;
}

bool BuildingGenerator::LoadBuilder(std::string lowDensityFile, std::string highDensityFile)
{
    return lowDensityBuildingBuilder.LoadTreeFromFile(lowDensityFile) && highDensityBuildingBuilder.LoadTreeFromFile(highDensityFile);
}

void BuildingGenerator::GetScaledModelPoints(std::vector<glm::vec3>& points, glm::vec3 scaleFactor, unsigned int modelId)
{
    for (unsigned int i = 0; i < modelManager->GetModel(modelId).vertices.positions.size(); i++)
    {
        points.push_back(modelManager->GetModel(modelId).vertices.positions[i] * scaleFactor);
    }
}

std::vector<Model> BuildingGenerator::GetRandomBuilding(DecisionTree<BuildingDecisionData>& builder, const glm::vec3& offset, float* separationRadius, float* height)
{
    std::vector<Model> resultingSegments;

    glm::vec3 overallScale = glm::vec3(1.0f);

    *height = 0.0f;

    std::vector<BuildingDecisionData> buildingSegmentRules = builder.EvaluateTreeSequence(BuildingGenerator::RandomWalkEvaluator);
    for (unsigned int i = 0; i < buildingSegmentRules.size(); i++)
    {
        const BuildingDecisionData& buildingRule = buildingSegmentRules[i];
        int layers = glm::linearRand(buildingRule.minLayers, buildingRule.maxLayers);
        while (layers > 0)
        {
            // For each segment, scale the model according to randomness and z-factors.
            float scaleFactor = glm::linearRand(buildingRule.minScaleFactor, buildingRule.maxScaleFactor);
            overallScale *= glm::vec3(scaleFactor);

            Model model = Model();
            model.scaleFactor = overallScale * glm::vec3(1.0f, 1.0f, buildingRule.zFactor);
            model.modelId = modelManager->GetModelId(buildingRule.modelName);

            std::vector<glm::vec3> scaledPoints;
            GetScaledModelPoints(scaledPoints, model.scaleFactor, model.modelId);
            btCollisionShape* collisionShape = new btConvexHullShape((btScalar*)&scaledPoints[0], scaledPoints.size(), sizeof(glm::vec3));

            btVector3 aabbMin;
            btVector3 aabbMax;
            collisionShape->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);

            float delta = 0.01;
            btVector3 buildingSegmentOrigin = btVector3(offset.x, offset.y, delta + offset.z + *height - aabbMin.z());
            if (i == 0)
            {
                // The building base is static.
                model.body = physics->GetStaticBody(collisionShape, buildingSegmentOrigin);
            }
            else
            {
                // TODO configurable mass.
                model.body = physics->GetDynamicBody(collisionShape, buildingSegmentOrigin, 400);
            }

            // Consumers must setup the analysis body themselves
            model.analysisBody = nullptr;
            model.body->setActivationState(ISLAND_SLEEPING);

            resultingSegments.push_back(model);
            *height += (delta + aabbMax.z() - aabbMin.z());
            --layers;
        }
    }

    *separationRadius = 10.0f * overallScale.x; // TODO configurable -- this is a model constant.
    Logger::Log("Generated building with ", resultingSegments.size(), " segments, from ", buildingSegmentRules.size(), " rules, with a separation radius of ", *separationRadius, ".");
    return resultingSegments;
}

// Same as the low-density, but with a high-density building.
std::vector<Model> BuildingGenerator::GetRandomHighDensityBuilding(glm::vec3 offset, float* buildingFootprintSize, float* height)
{
    return GetRandomBuilding(highDensityBuildingBuilder, offset, buildingFootprintSize, height);
}

// Returns a random low density building centered (XY) on the origin starting at Z == 0.
std::vector<Model> BuildingGenerator::GetRandomLowDensityBuilding(glm::vec3 offset, float* buildingFootprintSize, float* height)
{
    return GetRandomBuilding(lowDensityBuildingBuilder, offset, buildingFootprintSize, height);
}
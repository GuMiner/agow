#include <Bullet\btBulletDynamicsCommon.h>
#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "Utils\StringUtils.h"
#include "BuildingGenerator.h"

DecisionTree<BuildingDecisionData> BuildingGenerator::lowDensityBuildingBuilder(BuildingGenerator::DeserializeBuildingRule);
DecisionTree<BuildingDecisionData> BuildingGenerator::highDensityBuildingBuilder(BuildingGenerator::DeserializeBuildingRule);

BuildingGenerator::BuildingGenerator(ModelManager* modelManager, BasicPhysics* basicPhysics)
    : modelManager(modelManager), basicPhysics(basicPhysics)
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
        if (MathOps::Rand() > 0.50f)
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

    modelId = modelManager->LoadModel("models/buildings/segments/ell");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/segments/rect");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/segments/square");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/segments/tee");
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

    modelId = modelManager->LoadModel("models/buildings/tops/ell");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/flat");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/flatRect");
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

    modelId = modelManager->LoadModel("models/buildings/tops/pyramidRect");
    if (modelId == 0)
    {
        return false;
    }

    modelId = modelManager->LoadModel("models/buildings/tops/tee");
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

void BuildingGenerator::GetScaledModelPoints(std::vector<vec::vec3>& points, vec::vec3 scaleFactor, unsigned int modelId)
{
    for (unsigned int i = 0; i < modelManager->GetModel(modelId).vertices.positions.size(); i++)
    {
        points.push_back(modelManager->GetModel(modelId).vertices.positions[i] * scaleFactor);
    }
}

// Returns a random low density building centered (XY) on the origin starting at Z == 0.
std::vector<ScaledPhysicalModel> BuildingGenerator::GetRandomLowDensityBuilding(vec::vec3 offset, float* separationRadius)
{
    std::vector<ScaledPhysicalModel> resultingSegments;

    vec::vec3 overallScale = vec::vec3(1.0f);
    float currentHeight = 0.0f;
    *separationRadius = 0.0f;

    std::vector<BuildingDecisionData> buildingSegmentRules = lowDensityBuildingBuilder.EvaluateTreeSequence(BuildingGenerator::RandomWalkEvaluator);
    for (unsigned int i = 0; i < buildingSegmentRules.size(); i++)
    {
        const BuildingDecisionData& buildingRule = buildingSegmentRules[i];
        int layers = MathOps::Rand(buildingRule.minLayers, buildingRule.maxLayers + 1);
        while (layers > 0)
        {
            // For each segment, scale the model according to randomness and z-factors.
            float scaleFactor = MathOps::Rand() * (buildingRule.maxScaleFactor - buildingRule.minScaleFactor) + buildingRule.minScaleFactor;
            overallScale *= vec::vec3(scaleFactor);

            ScaledPhysicalModel model;
            model.scaleFactor = overallScale * vec::vec3(1.0f, 1.0f, buildingRule.zFactor);
            model.modelId = modelManager->GetModelId(buildingRule.modelName);

            std::vector<vec::vec3> scaledPoints;
            GetScaledModelPoints(scaledPoints, model.scaleFactor, model.modelId);
            btCollisionShape* collisionShape = new btConvexHullShape((btScalar*)&scaledPoints[0], scaledPoints.size(), sizeof(vec::vec3));
            
            btVector3 aabbMin;
            btVector3 aabbMax;
            collisionShape->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);

            btVector3 boundingSphereCenter;
            btScalar boundingSphereRadius;
            collisionShape->getBoundingSphere(boundingSphereCenter, boundingSphereRadius);

            *separationRadius = std::max(*separationRadius, boundingSphereRadius);

            btVector3 buildingSegmentOrigin = btVector3(offset.x, offset.y, offset.z + currentHeight - aabbMin.z());
            if (i == 0)
            {
                // The building base is static.
                model.rigidBody = basicPhysics->GetStaticBody(collisionShape, buildingSegmentOrigin);
            }
            else
            {
                // TODO configurable mass.
                model.rigidBody = basicPhysics->GetDynamicBody(collisionShape, buildingSegmentOrigin, 400);
            }

            model.rigidBody->setActivationState(ISLAND_SLEEPING);

            resultingSegments.push_back(model);
            currentHeight += aabbMax.z() - aabbMin.z();
            --layers;
        }
    }

    Logger::Log("Generated building with ", resultingSegments.size(), " segments, from ", buildingSegmentRules.size(), " rules, with a separation radius of ", *separationRadius, ".");
    return resultingSegments;
}
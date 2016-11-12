#include <algorithm>
#include <chrono>
#include <limits>
#include <glm/gtc/random.hpp>
#include "logging\Logger.h"
#include "TreeGenerator.h"

TreeGenerator::TreeGenerator()
{
}

bool TreeGenerator::IsDenseType(TreeType type)
{
    switch (type)
    {
    case SPHERE_DENSE:
    case POINTY_DENSE:
    case OVAL_DENSE:
    case SIDE_OVAL_DENSE:
    case RECT_DENSE:
        return true;
    default:
        return false;
    }
}

bool TreeGenerator::IsPointWithinShape(TreeType type, const glm::vec3& pos)
{
    const float ovalRadius = 0.25f; // TODO configurable.
    const float sideOvalRadius = 0.50f;

    switch (type)
    {
    case TreeType::RECT_DENSE:
    case TreeType::RECT_SPARSE:
        return true;
    case TreeType::SPHERE_DENSE:
    case TreeType::SPHERE_SPARSE:
        return glm::length(pos) < 1.0f;
    case TreeType::OVAL_DENSE:
    case TreeType::OVAL_SPARSE:
        return (pos.x * pos.x) + (pos.y * pos.y) + (pos.z * pos.z) / (ovalRadius * ovalRadius) < 1.0f;
    case TreeType::SIDE_OVAL_DENSE:
    case TreeType::SIDE_OVAL_SPARSE:
        return (pos.x * pos.x) / (sideOvalRadius * sideOvalRadius) + (pos.y * pos.y) / (sideOvalRadius * sideOvalRadius) + (pos.z * pos.z) < 1.0f;
    case TreeType::POINTY_DENSE:
    case TreeType::POINTY_SPARSE:
        return (pos.x * pos.x) + (pos.y * pos.y) < pow(1.0f - ((1.0f + pos.z) / 2.0f), 2.0f); // This forms a cone.
    default:
        return true;
    }
}

void TreeGenerator::GenerateAttractionPoints(TreeType type, float radius, float height, std::vector<glm::vec3>* points)
{
    // TODO configurable. All attraction points are above the trunk height.
    float trunkHeight = (0.11f + glm::linearRand(0.0f, 1.0f) * 0.22f) * height;
    float shapeHeight = height - trunkHeight; // Guaranteed to be positive.

    unsigned int pointCount = IsDenseType(type) ? 2000 : 1000;
    unsigned int maxIterations = 2000;
    for (unsigned int i = 0; points->size() <= pointCount && i < maxIterations; i++)
    {
        // Potentially generate a random point within the vincinity of the shape (cube of [-radius, -radius, 0], [radius, radius, shapeHeight])
        glm::vec3 point = glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f));
        if (IsPointWithinShape(type, point))
        {
            points->push_back(point * glm::vec3(radius * 2.0f - radius, radius * 2.0f - radius, shapeHeight) + glm::vec3(0, 0, trunkHeight));
        }
    }
}

float TreeGenerator::GetMinLeafDistance(glm::vec3 point, std::vector<Leaf>* leafs)
{
    float minDistance = std::numeric_limits<float>::max();
    for (unsigned int i = 0; i < leafs->size(); i++)
    {
        float distance = glm::length(point - (*leafs)[i].pos);
        if (distance < minDistance)
        {
            minDistance = distance;
        }
    }

    return minDistance;
}

void TreeGenerator::GrowTrunk(std::vector<Branch>* branches, std::vector<Leaf>* leafs, float branchLength, float leafDetectionDistance, float maxHeight)
{
    branches->push_back(Branch(-1, glm::vec3(0, 0, 0), glm::vec3(0, 0, branchLength)));
    bool lastBranchCloseEnough = GetMinLeafDistance((*branches)[branches->size() - 1].end(), leafs) < leafDetectionDistance;
    
    // Grow until we're ready to add leaves or our trunk is the height of the tree!
    while (!lastBranchCloseEnough && (*branches)[branches->size() - 1].pos.z < maxHeight)
    {
        Branch* parent = &((*branches)[branches->size() - 1]);
        branches->push_back(Branch(branches->size() - 1, parent->end(), parent->startDirection));

        lastBranchCloseEnough = GetMinLeafDistance((*branches)[branches->size() - 1].end(), leafs) < leafDetectionDistance;
    }
}

bool TreeGenerator::IsBranchWithinDistance(std::vector<Branch>* branches, Branch branch, float distance)
{
    for (unsigned int i = 0; i < branches->size(); i++)
    {
        if (glm::length(branch.pos - (*branches)[i].pos) < distance && 
            glm::length(branch.end() - (*branches)[i].end()) < distance)
        {
            return true;
        }
    }

    return false;
}

void TreeGenerator::FindInverseBranchSizes(unsigned int currentSize, Branch* branch)
{
    branch->thickness = currentSize;
    ++currentSize;

    for (unsigned int i = 0; i < branch->children.size(); i++)
    {
        FindInverseBranchSizes(currentSize, branch->children[i]);
    }
}

void TreeGenerator::InvertBranchSizes(std::vector<unsigned int>* branchSizes)
{
    unsigned int maxSize = 0;
    for (unsigned int i = 0; i < branchSizes->size(); i++)
    {
        if ((*branchSizes)[i] > maxSize)
        {
            maxSize = (*branchSizes)[i];
        }
    }

    // Invert but ensure the min branch size is 1.
    maxSize++;
    for (unsigned int i = 0; i < branchSizes->size(); i++)
    {
        (*branchSizes)[i] = maxSize - (*branchSizes)[i];
    }
}

// Generates a random tree.
GenerationResults TreeGenerator::GenerateTree(std::vector<glm::vec3>* trunkLines, std::vector<unsigned int>* trunkSizes, std::vector<glm::vec3>* leafPoints)
{
    TreeType type = (TreeType)glm::linearRand(0, (int)TreeType::COUNT - 1);
    
    // TODO configurable
    float radius = 2.0f + glm::linearRand(0.0f, 4.0f);
    float height = std::min(10.0f, radius + glm::linearRand(-1.0f, 8.0f));

    return GenerateTree(type, radius, height, trunkLines, trunkSizes, leafPoints);
}

// Generates a tree of the specified type.
GenerationResults TreeGenerator::GenerateTree(TreeType type, float radius, float height,
    std::vector<glm::vec3>* trunkLines, std::vector<unsigned int>* trunkSizes, std::vector<glm::vec3>* leafPoints)
{
    // TODO support trunk sizes by determining max parent height of branches.
    auto startTime = std::chrono::system_clock::now();

    // Generate the tree at the origin
    std::vector<glm::vec3> attractionPoints;
    GenerateAttractionPoints(type, radius, height, &attractionPoints);

    // TODO configurable
    float minDistance = 0.60f;
    float maxDistance = 0.90f;

    float branchLength = 0.20f;
    float branchClosenessLimit = 0.04f;

    unsigned int branchLimit = 1500;

    // At this point all the leaves are from -radius to +radius, 0 to height.
    std::vector<Leaf> leaves;
    for (unsigned int i = 0; i < attractionPoints.size(); i++)
    {
        leaves.push_back(Leaf(attractionPoints[i]));
    }

    std::vector<Branch> branches;
    GrowTrunk(&branches, &leaves, branchLength, maxDistance, height);

    // Now we are ready to run the space colonization-based tree algorithm.
    const int maxIterations = 2000;
    
    unsigned int leavesAdded = 0;
    bool branchesAdded = true;
    int iterations = 0;
    for (; iterations < maxIterations && 
           branchesAdded &&
           branches.size() < branchLimit;
           iterations++)
    {
        std::vector<unsigned int> leavesToRemove;
        for (unsigned int i = 0; i < leaves.size(); i++)
        {
            bool leafRemoved = false;
            leaves[i].closestBranch = nullptr;
            leaves[i].closestDistance = std::numeric_limits<float>::max();

            // Find the closest branch, remove the leaf if it is close enough.
            for (unsigned int j = 0; j < branches.size(); j++)
            {
                float distance = glm::length(branches[j].end() - leaves[i].pos);
                if (distance < minDistance)
                {
                    // The leaf is too close, so we remove it and add it to the known leaf points.
                    leafPoints->push_back(branches[j].pos + glm::linearRand(0.0f, 1.0f) * (branches[j].end() - leaves[i].pos));
                    leavesToRemove.push_back(i);
                    ++leavesAdded;
                    leafRemoved = true;
                    break;
                }
                else if (distance < maxDistance)
                {
                    // The leaf is close enough. Figure out if this is the closest branch for the leaf.
                    if (leaves[i].closestBranch == nullptr || distance < leaves[i].closestDistance)
                    {
                        leaves[i].closestBranch = &branches[j];
                        leaves[i].closestDistance = distance;
                    }
                }
            }

            // Tug the branch towards the leaf accordingly. Tug is *regardless of distance from branch*.
            if (!leafRemoved && leaves[i].closestBranch != nullptr)
            {
                glm::vec3 direction = leaves[i].pos - leaves[i].closestBranch->end();
                leaves[i].closestBranch->growDirection += (glm::normalize(direction) * branchLength);
                leaves[i].closestBranch->grew++;
            }
        }

        // Erase all leaves we decided to erase.
        for (int i = leavesToRemove.size() - 1; i >= 0; i--)
        {
            leaves.erase(leaves.begin() + leavesToRemove[i]);
        }

        // Create new branches.
        std::vector<Branch> newBranches;
        for (unsigned int i = 0; i < branches.size(); i++)
        {
            if (branches[i].grew != 0)
            {
                glm::vec3 averageDirection = glm::normalize(branches[i].growDirection);
                newBranches.push_back(Branch(i, branches[i].end(), averageDirection * branchLength * (float)(1 + branches[i].grew % 3)));

                branches[i].growDirection = branches[i].startDirection;
                branches[i].grew = 0;
            }
        }

        // Add new branches if they're not too close together.
        branchesAdded = false;
        for (unsigned int i = 0; i < newBranches.size(); i++)
        {
            if (IsBranchWithinDistance(&branches, newBranches[i], branchClosenessLimit))
            {
                continue;
            }
            else
            {
                branches.push_back(newBranches[i]);
                branchesAdded = true;
            }
        }
    }

    // Now that we're done with the main loop:
    
    // Log tree generation results.
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    Logger::Log("Tree Gen: ", (int)(duration.count() * 1000000.0f), " us, A: ", attractionPoints.size(), " I: ", iterations, " L: ", leavesAdded, " B: ", branches.size());

    // Populate the children list.
    for (unsigned int i = 0; i < branches.size(); i++)
    {
        if (branches[i].parentId >= 0)
        {
            branches[branches[i].parentId].children.push_back(&(branches[i]));
        }
    }

    // Determine the trunk sizes recursively.
    FindInverseBranchSizes(1, &branches[0]);

    // Add the branches as trunk lines; note the leaves have already been added.
    for (unsigned int i = 0; i < branches.size(); i++)
    {
        trunkLines->push_back(branches[i].pos);
        trunkLines->push_back(branches[i].end());
        trunkSizes->push_back(branches[i].thickness);
        trunkSizes->push_back(branches[i].thickness);
    }

    // All our branch sizes are reversed, so perform the inverse of that operation.
    InvertBranchSizes(trunkSizes);
    
    return GenerationResults(type, leavesAdded, branches.size());
}
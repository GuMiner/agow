#pragma once
#include <vector>
#include "Math\Vec.h"

struct Branch
{
    Branch* parent;
    vec::vec3 pos;
    vec::vec3 startDirection;
    
    vec::vec3 growDirection;
    bool grew;

    Branch(Branch* parent, vec::vec3 pos, vec::vec3 startDirection)
        : parent(parent), pos(pos), startDirection(startDirection), growDirection(startDirection), grew(false)
    {
    }
};

struct Leaf
{
    vec::vec3 pos;
    
    float closestDistance;
    Branch* closestBranch;

    Leaf(vec::vec3 pos)
        : pos(pos), closestDistance(std::numeric_limits<float>::max())
    {
    }
};

enum TreeType
{
    SPHERE_SPARSE,
    SPHERE_DENSE,
    POINTY_SPARSE,
    POINTY_DENSE,
    OVAL_SPARSE,
    OVAL_DENSE,
    SIDE_OVAL_SPARSE,
    SIDE_OVAL_DENSE,
    RECT_SPARSE,
    RECT_DENSE,
    COUNT
};

struct GenerationResults
{
    TreeType type;
    unsigned int attractionPoints;
    unsigned int leaves;
    unsigned int branches;

    GenerationResults(TreeType type, unsigned int attractionPoints, unsigned int leaves, unsigned int branches)
        : type(type), attractionPoints(attractionPoints), leaves(leaves), branches(branches)
    {
    }
};

class TreeGenerator
{
    bool IsDenseType(TreeType type);

    // Checks if the point is within a shape (0 to 1 in all dimensions).
    bool IsPointWithinShape(TreeType type, const vec::vec3& pos);

    void GenerateAttractionPoints(TreeType type, float radius, float height, std::vector<vec::vec3>* points);

    float GetMinLeafDistance(vec::vec3 point, std::vector<Leaf>* leafs);
    void GrowTrunk(std::vector<Branch>* branches, std::vector<Leaf>* leafs, float branchLength, float leafDetectionDistance, float maxHeight);

    bool IsBranchWithinDistance(std::vector<Branch>* branches, vec::vec3 point, float distance);

public:
    TreeGenerator();

    // Generates a random tree.
    GenerationResults GenerateTree(const vec::vec3& pos, std::vector<vec::vec3>* trunkLines, std::vector<unsigned int>* trunkSizes, std::vector<vec::vec3>* leafPoints);

    // Generates a tree of the specified type.
    GenerationResults GenerateTree(TreeType type, const vec::vec3& pos, float radius, float height,
        std::vector<vec::vec3>* trunkLines, std::vector<unsigned int>* trunkSizes, std::vector<vec::vec3>* leafPoints);
};


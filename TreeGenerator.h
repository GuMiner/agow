#pragma once
#include <vector>
#include <glm\vec3.hpp>

struct Branch
{
    Branch* parent;
    glm::vec3 pos;
    glm::vec3 startDirection;
    
    glm::vec3 growDirection;
    unsigned int grew;

    Branch(Branch* parent, glm::vec3 pos, glm::vec3 startDirection)
        : parent(parent), pos(pos), startDirection(startDirection), growDirection(startDirection), grew(0)
    {
    }

    glm::vec3 end() const
    {
        return pos + startDirection;
    }
};

struct Leaf
{
    glm::vec3 pos;
    
    float closestDistance;
    Branch* closestBranch;

    Leaf(glm::vec3 pos)
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
    unsigned int leaves;
    unsigned int branches;

    GenerationResults(TreeType type, unsigned int leaves, unsigned int branches)
        : type(type), leaves(leaves), branches(branches)
    {
    }
};

class TreeGenerator
{
    bool IsDenseType(TreeType type);

    // Checks if the point is within a shape (0 to 1 in all dimensions).
    bool IsPointWithinShape(TreeType type, const glm::vec3& pos);

    void GenerateAttractionPoints(TreeType type, float radius, float height, std::vector<glm::vec3>* points);

    float GetMinLeafDistance(glm::vec3 point, std::vector<Leaf>* leafs);
    void GrowTrunk(std::vector<Branch>* branches, std::vector<Leaf>* leafs, float branchLength, float leafDetectionDistance, float maxHeight);

    bool IsBranchWithinDistance(std::vector<Branch>* branches, Branch branch, float distance);

public:
    TreeGenerator();

    // Generates a random tree.
    GenerationResults GenerateTree(const glm::vec3& pos, std::vector<glm::vec3>* trunkLines, std::vector<unsigned int>* trunkSizes, std::vector<glm::vec3>* leafPoints);

    // Generates a tree of the specified type.
    GenerationResults GenerateTree(TreeType type, const glm::vec3& pos, float radius, float height,
        std::vector<glm::vec3>* trunkLines, std::vector<unsigned int>* trunkSizes, std::vector<glm::vec3>* leafPoints);
};


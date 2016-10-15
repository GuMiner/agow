#pragma once
#include <map>
#include <vector>
#include <glm\vec2.hpp>
#include "Data\Model.h"
#include "Managers\ImageManager.h"
#include "Utils\Vertex.h"

struct PosUvPair
{
    unsigned int positionId;
    unsigned int uvId;
};

class ModelLoader
{
    ImageManager* imageManager;

    // Temporary loading structures.
    std::vector<glm::vec2> rawUvs;
    std::vector<PosUvPair> rawIndices;
    std::map<unsigned int, unsigned int> indexUvMap; // [positionId] = UV Id. Maps rawIndices for faster loading access.
    std::map<unsigned int, std::vector<PosUvPair>> uvVertexRemapping;

    // Given a position index and UV coordinate index, returns the actual index to use.
    unsigned int GetActualVertexIndex(unsigned int positionIdx, unsigned int uvIdx, universalVertices& vertices);

    // Parses an individual line of an OBJ model file.
    bool ParseLine(const std::vector<std::string>& line, universalVertices& vertices);

    // Loads an OBJ model into the specified vertices, returning true on success.
    // Note that the OBJ model must fully specify all positions / UVs *before* any indices.
    bool LoadModel(const char* objFilename, universalVertices& vertices, int* rawPointCount, glm::vec3* minBounds, glm::vec3* maxBounds);

public:
    ModelLoader(ImageManager* imageManager);

    // Loads the specified texture model, returning true on success.
    bool LoadModel(const char* rootFilename, TextureModel* loadedModel);
};
#include <sstream>
#include "Utils\Logger.h"
#include "Utils\StringUtils.h"
#include "ModelLoader.h"

ModelLoader::ModelLoader(ImageManager* imageManager)
    : imageManager(imageManager)
{
}

// Loads the specified texture model, returning true on success.
bool ModelLoader::LoadModel(const char* rootFilename, TextureModel* loadedModel)
{

    std::stringstream combinationStream;
    combinationStream << rootFilename << ".png";
    std::string pngString = combinationStream.str();

    combinationStream.str("");
    combinationStream << rootFilename << ".obj";
    std::string objString = combinationStream.str();

    loadedModel->name = std::string(rootFilename);
    loadedModel->textureId = imageManager->AddImage(pngString.c_str());
    if (loadedModel->textureId == 0)
    {
        Logger::Log("Error loading the texture image!");
        Logger::LogError(pngString.c_str());
        return false;
    }

    if (!LoadModel(objString.c_str(), loadedModel->vertices, &loadedModel->rawPointCount, &loadedModel->minBounds, &loadedModel->maxBounds))
    {
        Logger::Log("Error loading the OBJ model!");
        Logger::LogError(objString.c_str());
        return false;
    }

    return true;
}

unsigned int ModelLoader::GetActualVertexIndex(unsigned int positionIdx, unsigned int uvIdx, universalVertices& vertices)
{
    for (const PosUvPair& pair : uvVertexRemapping[positionIdx])
    {
        if (pair.uvId == uvIdx)
        {
            // Remapping already exists.
            return pair.positionId;
        }
    }

    // Remappings exist, but the UV coordinate listed isn't there. Add a new remapping.
    PosUvPair newVertexRemapping;
    newVertexRemapping.uvId = uvIdx;
    newVertexRemapping.positionId = vertices.positions.size();
    uvVertexRemapping[positionIdx].push_back(newVertexRemapping);

    // Duplicate the vertex -- but with the new UV coordinate
    const PosUvPair& validMapping = uvVertexRemapping[positionIdx][0];
    vertices.positions.push_back(vertices.positions[validMapping.positionId]);
    vertices.uvs.push_back(rawUvs[uvIdx]);

    return newVertexRemapping.positionId;
}

bool ModelLoader::ParseLine(const std::vector<std::string>& line, universalVertices& vertices)
{
    // Skip tiny lines.
    if (line.size() < 3)
    {
        return true;
    }

    if (strncmp(line[0].c_str(), "v", 1) == 0 && line[0].size() == 1)
    {
        // Positions
        if (line.size() != 4)
        {
            Logger::Log("Line did not have 3 elements for a vertex position.");
            return false;
        }

        glm::vec3 vector;
        if (!StringUtils::ParseFloatFromString(line[1], vector.x) ||
            !StringUtils::ParseFloatFromString(line[2], vector.y) ||
            !StringUtils::ParseFloatFromString(line[3], vector.z))
        {
            Logger::Log("Line had an improper floating-point value.");
            return false;
        }

        vertices.positions.push_back(vector);
    }
    else if (strncmp(line[0].c_str(), "vt", 2) == 0 && line[0].size() == 2)
    {
        // UV coordinates
        if (line.size() != 3)
        {
            Logger::Log("Line did not have 2 elements for a UV coordinate.");
            return false;
        }

        glm::vec2 vector;
        if (!StringUtils::ParseFloatFromString(line[1], vector.x) ||
            !StringUtils::ParseFloatFromString(line[2], vector.y))
        {
            Logger::Log("Line had an improper floating-point value.");
            return false;
        }

        rawUvs.push_back(vector);
    }
    else if (strncmp(line[0].c_str(), "f", 1) == 0 && line[0].size() == 1)
    {
        // Indices
        if (line.size() != 4)
        {
            Logger::Log("Line did not have 3 elements for an index mapping.");
            return false;
        }

        // Indicies can be processed individually.
        for (unsigned int i = 0; i < 3; i++)
        {
            std::vector<std::string> separatedIndices;
            StringUtils::Split(line[i + 1], '/', true, separatedIndices);
            if (separatedIndices.size() != 2)
            {
                Logger::Log("Line did not have a position and UV coordinate for a portion of an index mapping.");
                return false;
            }

            int positionIndex, uvIndex;
            if (!StringUtils::ParseIntFromString(separatedIndices[0], positionIndex) ||
                !StringUtils::ParseIntFromString(separatedIndices[1], uvIndex))
            {
                Logger::Log("Index had an improper integer value.");
                return false;
            }

            // Indices should be zero-based.
            positionIndex--;
            uvIndex--;

            PosUvPair pair;
            pair.positionId = (unsigned int)positionIndex;
            pair.uvId = (unsigned int)uvIndex;

            rawIndices.push_back(pair);
            indexUvMap[pair.positionId] = pair.uvId;
        }
    }

    return true;
}

bool ModelLoader::LoadModel(const char* objFilename, universalVertices& vertices, int* rawPointCount, glm::vec3* minBounds, glm::vec3* maxBounds)
{
    std::string fileString;
    if (!StringUtils::LoadStringFromFile(objFilename, fileString))
    {
        Logger::Log("Could not read the OBJ file!");
        return false;
    }

    std::vector<std::string> fileLines;
    StringUtils::Split(fileString, StringUtils::Newline, true, fileLines);

    rawIndices.clear();
    indexUvMap.clear();
    rawUvs.clear();
    for (const std::string& line : fileLines)
    {
        std::vector<std::string> splitLine;
        StringUtils::Split(line, StringUtils::Space, true, splitLine);
        if (!ParseLine(splitLine, vertices))
        {
            Logger::Log("Error parsing line in OBJ file!");
            Logger::Log(line.c_str());
            return false;
        }
    }

    *rawPointCount = vertices.positions.size();

    // Properly render out the vertices so that there is one UV per vertex.
    uvVertexRemapping.clear();

    // Also figure out the min-max bounding box while we're at it.
    minBounds->x = std::numeric_limits<float>::max();
    minBounds->y = std::numeric_limits<float>::max();
    minBounds->z = std::numeric_limits<float>::max();
    maxBounds->x = std::numeric_limits<float>::min();
    maxBounds->y = std::numeric_limits<float>::min();
    maxBounds->z = std::numeric_limits<float>::min();

    // There's guaranteed to be a UV for each point. Find it, and set it.
    for (unsigned int i = 0; i < vertices.positions.size(); i++)
    {
        if (indexUvMap.find(i) != indexUvMap.end())
        {
            PosUvPair pair;
            pair.positionId = i;
            pair.uvId = indexUvMap[i];
            vertices.uvs.push_back(rawUvs[pair.uvId]);

            // Initialize the remapping for this index.
            uvVertexRemapping[i] = std::vector<PosUvPair>();
            uvVertexRemapping[i].push_back(pair);
        }
        else
        {
            std::stringstream errorStream;
            errorStream << "Failed to load in the UV for point " << i << ".";
            Logger::Log(errorStream.str().c_str());
            return false;
        }

        // Perform min-max bounding check.
        if (vertices.positions[i].x < minBounds->x)
        {
            minBounds->x = vertices.positions[i].x;
        }

        if (vertices.positions[i].y < minBounds->y)
        {
            minBounds->y = vertices.positions[i].y;
        }

        if (vertices.positions[i].z < minBounds->z)
        {
            minBounds->z = vertices.positions[i].z;
        }

        if (vertices.positions[i].x > maxBounds->x)
        {
            maxBounds->x = vertices.positions[i].x;
        }

        if (vertices.positions[i].y > maxBounds->y)
        {
            maxBounds->y = vertices.positions[i].y;
        }

        if (vertices.positions[i].z > maxBounds->z)
        {
            maxBounds->z = vertices.positions[i].z;
        }
    }

    // Now parse out the indices, as we can properly use the uvVertexRemapping list.
    for (const PosUvPair& pair : rawIndices)
    {
        unsigned int vertexIndex = GetActualVertexIndex(pair.positionId, pair.uvId, vertices);
        vertices.indices.push_back(vertexIndex);
    }

    return true;
}
#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

TerrainManager::TerrainManager(glm::ivec2 min, glm::ivec2 max, ShaderManager* shaderManager, ModelManager* modelManager, BasicPhysics* basicPhysics, std::string terrainRootFolder, int tileSize)
    : min(min), max(max), shaderManager(shaderManager), rootFolder(terrainRootFolder), tileSize(tileSize), terrainEffects(shaderManager, modelManager, basicPhysics, tileSize / TerrainManager::Subdivisions)
{
}

int TerrainManager::GetTileSize() const
{
    return tileSize;
}

int TerrainManager::GetSubTileSize() const
{
    return GetTileSize() / TerrainManager::Subdivisions;
}

bool TerrainManager::LoadBasics()
{
    if (!shaderManager->CreateShaderProgramWithTesselation("terrainRender", &terrainRenderProgram))
    {
        Logger::LogError("Failed to load the basic terrain rendering shader; cannot continue.");
        return false;
    }

    terrainTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainTexture");
    terrainTypeTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainType");
    gameTimeLocation = glGetUniformLocation(terrainRenderProgram, "gameTime");
    projLocation = glGetUniformLocation(terrainRenderProgram, "projMatrix");
    mvLocation = glGetUniformLocation(terrainRenderProgram, "mvMatrix");

    if (!terrainEffects.LoadBasics())
    {
        Logger::LogError("Failed to load the terrain effects initial setup; cannot continue.");
        return false;
    }

    return true;
}

GLuint TerrainManager::CreateTileTexture(GLenum activeTexture, int subSize, float* heightmap)
{
    GLuint newTextureId;
    glGenTextures(1, &newTextureId);

    glActiveTexture(activeTexture);
    glBindTexture(GL_TEXTURE_2D, newTextureId);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, subSize, subSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, subSize, subSize, GL_RED, GL_FLOAT, heightmap);

    // Ensure we clamp to the edges to avoid border problems.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return newTextureId;
}

GLuint TerrainManager::CreateTileTexture(GLenum activeTexture, int subSize, unsigned char* heightmap)
{
    GLuint newTextureId;
    glGenTextures(1, &newTextureId);

    glActiveTexture(activeTexture);
    glBindTexture(GL_TEXTURE_2D, newTextureId);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, subSize, subSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, subSize, subSize, GL_RED, GL_UNSIGNED_BYTE, heightmap);

    // Ensure we clamp to the edges to avoid border problems.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return newTextureId;
}

bool TerrainManager::LoadTileToCache(glm::ivec2 start, bool loadSubtiles)
{
    bool loadedImage = false;
    if (terrainTiles.find(start) != terrainTiles.end())
    {
        // Tile is already in the cache -- but if we want subtiles to be loaded and they haven't been, continue.
        loadedImage = true;
        if (terrainTiles[start]->loadedSubtiles || terrainTiles[start]->loadedSubtiles == loadSubtiles)
        {
            return true;
        }
    }
    else
    {
        terrainTiles[start] = new TerrainTile();
        terrainTiles[start]->loadedSubtiles = false;
    }

    if (!loadedImage)
    {
        std::stringstream tileName;
        tileName << rootFolder << "/" << start.y << "/" << start.x << ".png";

        int width, height;
        if (!ImageUtils::GetRawImage(tileName.str().c_str(), &terrainTiles[start]->rawImage, &width, &height) || width != tileSize || height != tileSize)
        {
            Logger::Log("Failed to load tile [", start.x, ", ", start.y, "] because of bad image/width/height: [", width, ", ", height, ".");
            return false;
        }
    }

    if (loadSubtiles && !terrainTiles[start]->loadedSubtiles)
    {
        int subSize = GetSubTileSize() + 2;
        for (int i = 0; i < TerrainManager::Subdivisions; i++)
        {
            for (int j = 0; j < TerrainManager::Subdivisions; j++)
            {
                // Populate our main image
                float* heightmap = new float[subSize * subSize];
                for (int x = 0; x < GetSubTileSize(); x++)
                {
                    for (int y = 0; y < GetSubTileSize(); y++)
                    {
                        int xReal = x + i * GetSubTileSize();
                        int yReal = y + j * GetSubTileSize();
                        
                        heightmap[(x + 1) + (y + 1) * subSize] =
                            (float)((unsigned short)terrainTiles[start]->rawImage[(xReal + yReal * tileSize) * 4] +
                                  (((unsigned short)terrainTiles[start]->rawImage[(xReal + yReal * tileSize) * 4 + 1]) << 8))
                                / (float)std::numeric_limits<unsigned short>::max();
                    }
                }

                // Populate the top and bottom band.
                for (int x = 1; x < subSize - 1; x++)
                {
                    // TODO unlike the last version, this should check the current tile first, and if we go OOB, then the next terrain tile.
                    heightmap[x + 0 * subSize] = heightmap[x + 1 * subSize];
                    heightmap[x + (subSize - 1) * subSize] = heightmap[x + (subSize - 2) * subSize];
                }

                for (int y = 1; y < subSize - 1; y++)
                {
                    heightmap[0 + y * subSize] = heightmap[1 + y * subSize];
                    heightmap[(subSize - 1) + y * subSize] = heightmap[(subSize - 2) + y * subSize];
                }

                // Fill in corners with inner corners.
                heightmap[0 + 0 * subSize] = heightmap[1 + 1 * subSize];
                heightmap[(subSize - 1) + (subSize - 1) * subSize] = heightmap[(subSize - 2) + (subSize - 2) * subSize];
                heightmap[(0) + (subSize - 1) * subSize] = heightmap[(1) + (subSize - 2) * subSize];
                heightmap[(subSize - 1) + (0) * subSize] = heightmap[(subSize - 2) + (1) * subSize];

                unsigned char* types = new unsigned char[GetSubTileSize() * GetSubTileSize()];
                for (int x = 0; x < GetSubTileSize(); x++)
                {
                    for (int y = 0; y < GetSubTileSize(); y++)
                    {
                        // Within our main image.
                        int xReal = x + i * GetSubTileSize();
                        int yReal = y + j * GetSubTileSize();

                        types[x + y * GetSubTileSize()] = terrainTiles[start]->rawImage[(xReal + yReal * tileSize) * 4 + 2];

                        // // Perform per-tile height modifications
                        // if (types[x + y * GetSubTileSize()] == TerrainTypes::ROADS)
                        // {
                        //     heightmap[(x + 1) + (y + 1) * subSize] -= (0.50f / 900.0f);
                        // }
                    }
                }

                GLuint heightmapTextureId = CreateTileTexture(GL_TEXTURE0, subSize, heightmap);
                GLuint typeTextureId = CreateTileTexture(GL_TEXTURE1, GetSubTileSize(), types);

                // After saving to OpenGL, scale accordingly for Bullet physics to properly deal with the terrain and remove the extra layer.
                float* realHeightmap = new float[GetSubTileSize() * GetSubTileSize()];
                const float scale = 900.0f;
                for (int x = 0; x < GetSubTileSize(); x++)
                {
                    for (int y = 0; y < GetSubTileSize(); y++)
                    {
                        realHeightmap[x + y * GetSubTileSize()] = scale * heightmap[(x + 1) + (y + 1) * subSize];
                    }
                }

                // We no longer need the data as it is now in the GPU.
                delete[] heightmap;

                glm::ivec2 subTilePos = glm::ivec2(i, j);
                terrainTiles[start]->subtiles[subTilePos] = new SubTile(heightmapTextureId, realHeightmap, typeTextureId, types);
                terrainEffects.LoadSubTileEffects(subTilePos + start * TerrainManager::Subdivisions, terrainTiles[start]->subtiles[subTilePos]);
            }
        }

        terrainTiles[start]->loadedSubtiles = true;
    }

    Logger::Log("Loading region tile (", start.x, ", ", start.y, ") succeeded.");
    return true;
}

bool TerrainManager::LoadTerrainTile(glm::ivec2 start, TerrainTile** tile)
{
    // The surrounding tiles must also be loaded to cache to properly generate subtile heightmaps and images.
    if (!LoadTileToCache(glm::ivec2(std::min(start.x + 1, max.x), start.y), false) ||
        !LoadTileToCache(glm::ivec2(std::max(start.x - 1, min.x), start.y), false) ||
        !LoadTileToCache(glm::ivec2(start.x, std::min(start.y + 1, max.y)), false) ||
        !LoadTileToCache(glm::ivec2(start.x, std::max(start.y - 1, min.y)), false) ||
        !LoadTileToCache(start, true))
    {
        return false;
    }

    *tile = terrainTiles[start];
    return true;
}

void TerrainManager::Update(float gameTime)
{
    lastGameTime = gameTime;
}

void TerrainManager::Simulate(const glm::ivec2 start, const glm::ivec2 subPos, float elapsedSeconds)
{
    if (terrainTiles.find(start) == terrainTiles.end())
    {
        Logger::LogWarn("Attempted to simulate a terrain tile not loaded with [", start.x, ", ", start.y, "].");
        return;
    }
    else if (terrainTiles[start]->subtiles.find(subPos) == terrainTiles[start]->subtiles.end())
    {
        Logger::LogWarn("Attempted to simulate a subtile that is invalid on [", start.x, ", ", start.y, "], subtile [", subPos.x, ", ", subPos.y, "].");
        return;
    }

    terrainEffects.Simulate(subPos + start * TerrainManager::Subdivisions, elapsedSeconds);
}

// TODO go everywhere else and cleanup projection / perspective / model / mv / view to all be correct.
void TerrainManager::RenderTile(const glm::ivec2 start, const glm::ivec2 subPos, const glm::mat4& perspectiveMatrix, const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
{
    if (terrainTiles.find(start) == terrainTiles.end())
    {
        Logger::LogWarn("Attempted to render a terrain tile not loaded with [", start.x, ", ", start.y, "].");
        return; 
    }
    else if (terrainTiles[start]->subtiles.find(subPos) == terrainTiles[start]->subtiles.end())
    {
        Logger::LogWarn("Attempted to render a subtile that is invalid on [", start.x, ", ", start.y, "], subtile [", subPos.x, ", ", subPos.y, "].");
        return;
    }

    // Render the tile.
    glUseProgram(terrainRenderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTiles[start]->subtiles[subPos]->heightmapTextureId);
    glUniform1i(terrainTexLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, terrainTiles[start]->subtiles[subPos]->typeTextureId);
    glUniform1i(terrainTypeTexLocation, 1);

    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &perspectiveMatrix[0][0]);
    glUniformMatrix4fv(mvLocation, 1, GL_FALSE, &(viewMatrix * modelMatrix)[0][0]);

    glUniform1f(gameTimeLocation, lastGameTime);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, GetSubTileSize() * GetSubTileSize());

    // Render tile SFX.
    terrainEffects.RenderSubTileEffects(subPos + start * TerrainManager::Subdivisions, perspectiveMatrix, viewMatrix, modelMatrix);
}

void TerrainManager::CleanupTerrainTile(glm::ivec2 start, bool log)
{
    // We need to delete all of the subtiles and then the tile itself.
    for (std::pair<const glm::ivec2, SubTile*> subTile : terrainTiles[start]->subtiles)
    {
        glDeleteTextures(1, &subTile.second->heightmapTextureId);
        delete[] subTile.second->heightmap;
        glDeleteTextures(1, &subTile.second->typeTextureId);
        delete[] subTile.second->type;

        delete subTile.second;
    }
    
    ImageUtils::FreeRawImage(terrainTiles[start]->rawImage);
    delete terrainTiles[start];

    if (log)
    {
        Logger::Log("Unloaded tile ", start.x, ", ", start.y, ".");
    }
}

void TerrainManager::UnloadTerrainTile(glm::ivec2 start)
{
    for (std::pair<const glm::ivec2, SubTile*> subTile : terrainTiles[start]->subtiles)
    {
        terrainEffects.UnloadSubTileEffects(start * TerrainManager::Subdivisions + subTile.first);
    }

    CleanupTerrainTile(start, true);
    terrainTiles.erase(start);
}

TerrainManager::~TerrainManager()
{
    // Cleanup any allocated terrain tiles.
    for (auto iter = terrainTiles.begin(); iter != terrainTiles.end(); iter++)
    {
        CleanupTerrainTile(iter->first, false);
    }
}

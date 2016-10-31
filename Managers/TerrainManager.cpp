#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

TerrainManager::TerrainManager(glm::ivec2 min, glm::ivec2 max, ShaderManager* shaderManager, ModelManager* modelManager, Physics* Physics, std::string terrainRootFolder)
    : min(min), max(max), shaderManager(shaderManager), rootFolder(terrainRootFolder), terrainEffects(shaderManager, modelManager, Physics), terrainRenderProgram(0)
{
}

TerrainEffectManager& TerrainManager::GetEffectManager()
{
    return terrainEffects;
}

bool TerrainManager::LoadBasics()
{
    if (!ReloadTerrainShader())
    {
        return false;
    }

    if (!terrainEffects.LoadBasics())
    {
        Logger::LogError("Failed to load the terrain effects initial setup; cannot continue.");
        return false;
    }

    return true;
}

bool TerrainManager::ReloadTerrainShader()
{
    GLuint shaderProgram;
    if (!shaderManager->CreateShaderProgramWithTesselation("terrainRender", &shaderProgram))
    {
        Logger::LogError("Failed to load the basic terrain rendering shader; cannot continue.");
        return false;
    }

    if (terrainRenderProgram != 0)
    {
        // Unload so we can attempt a reload.
        glDeleteProgram(terrainRenderProgram);
        terrainRenderProgram = 0;
    }

    terrainRenderProgram = shaderProgram;
    terrainTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainTexture");
    terrainTypeTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainType");
    gameTimeLocation = glGetUniformLocation(terrainRenderProgram, "gameTime");
    projLocation = glGetUniformLocation(terrainRenderProgram, "projMatrix");
    mvLocation = glGetUniformLocation(terrainRenderProgram, "mvMatrix");

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
        if (!ImageUtils::GetRawImage(tileName.str().c_str(), &terrainTiles[start]->rawImage, &width, &height) || width != TerrainTile::TileSize || height != TerrainTile::TileSize)
        {
            Logger::Log("Failed to load tile [", start.x, ", ", start.y, "] because of bad image/width/height: [", width, ", ", height, ".");
            return false;
        }
    }

    if (loadSubtiles && !terrainTiles[start]->loadedSubtiles)
    {
        int subSize = TerrainTile::SubtileSize + 2;
        for (int i = 0; i < TerrainTile::Subdivisions; i++)
        {
            for (int j = 0; j < TerrainTile::Subdivisions; j++)
            {
                // Populate our main image
                float* heightmap = new float[subSize * subSize];
                unsigned char* types = new unsigned char[subSize * subSize];
                for (int x = 0; x < TerrainTile::SubtileSize; x++)
                {
                    for (int y = 0; y < TerrainTile::SubtileSize; y++)
                    {   
                        int largerTilePixelId = (x + 1) + (y + 1) * subSize;
                        ReadTilePixel(start, glm::ivec2(x + i * TerrainTile::SubtileSize, y + j * TerrainTile::SubtileSize), &heightmap[largerTilePixelId], &types[largerTilePixelId]);
                    }
                }

                // Fill in edges and corners to avoid artifacts.
                LoadHeightmapEdges(start, i, j, subSize, heightmap, types);

                // Normalize lakes
                for (int x = 0; x < subSize; x++)
                {
                    for (int y = 0; y < subSize; y++)
                    {
                        // Perform per-tile height modifications
                        switch (types[x + y * subSize])
                        {
                        case TerrainTypes::ROADS:
                        case TerrainTypes::RIVER:
                        case TerrainTypes::CITY:
                        case TerrainTypes::DIRTLAND:
                        case TerrainTypes::GRASSLAND:
                        case TerrainTypes::ROCKS:
                        case TerrainTypes::SAND:
                        case TerrainTypes::SNOW_PEAK:
                        case TerrainTypes::TREES:
                            break;
                        default:
                            // Lake isn't guaranteed to be the last item, 
                            types[x + y * subSize] = TerrainTypes::LAKE;
                            break;
                        }
                    }
                }

                for (int x = 0; x < subSize; x++)
                {
                    for (int y = 0; y < subSize; y++)
                    {
                        int xMinus = std::max(0, --x) + y * subSize;
                        int xPlus = std::min(subSize - 1, ++x) + y * subSize;
                        int yMinus = x + std::max(0, --y) * subSize;
                        int yPlus = x + std::min(subSize - 1, ++y) * subSize;

                        // Perform per-tile height modifications
                        switch (types[x + y * subSize])
                        {
                        case TerrainTypes::ROADS:
                            heightmap[x + y * subSize] -= (0.50f / 900.0f);
                            break;
                        case TerrainTypes::RIVER:
                            if (types[xMinus] == TerrainTypes::RIVER &&
                                types[xPlus] == TerrainTypes::RIVER &&
                                types[yMinus] == TerrainTypes::RIVER &&
                                types[yPlus] == TerrainTypes::RIVER)
                            {
                                heightmap[x + y * subSize] -= (1.0f / 900.0f);
                            }
                            break;
                        case TerrainTypes::LAKE:
                            if (types[xMinus] == TerrainTypes::LAKE &&
                                types[xPlus] == TerrainTypes::LAKE &&
                                types[yMinus] == TerrainTypes::LAKE &&
                                types[yPlus] == TerrainTypes::LAKE)
                            {
                                heightmap[x + y * subSize] -= (2.0f / 900.0f);
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }

                // Send heightmap to OpenGL with buffer space.
                GLuint heightmapTextureId = CreateTileTexture(GL_TEXTURE0, subSize, heightmap);

                // After saving to OpenGL, scale accordingly for Bullet physics to properly deal with the terrain (heightmap only) and remove the extra layer (heightmap and type).
                float* realHeightmap = new float[TerrainTile::SubtileSize * TerrainTile::SubtileSize];
                unsigned char* realTypes = new unsigned char[TerrainTile::SubtileSize * TerrainTile::SubtileSize];
                const float scale = 900.0f;
                for (int x = 0; x < TerrainTile::SubtileSize; x++)
                {
                    for (int y = 0; y < TerrainTile::SubtileSize; y++)
                    {
                        int id = x + y * TerrainTile::SubtileSize;
                        int largerId = (x + 1) + (y + 1) * subSize;
                        realHeightmap[id] = scale * heightmap[largerId];
                        realTypes[id] = types[largerId];
                    }
                }

                // Send types to OpenGL with *no* buffer space.
                GLuint typeTextureId = CreateTileTexture(GL_TEXTURE1, TerrainTile::SubtileSize, realTypes);

                // We no longer need the data as it is now in the GPU.
                delete[] heightmap;
                delete[] types;

                glm::ivec2 subTilePos = glm::ivec2(i, j);
                terrainTiles[start]->subtiles[subTilePos] = new SubTile(heightmapTextureId, realHeightmap, typeTextureId, realTypes);
                terrainEffects.LoadSubTileEffects(subTilePos + start * TerrainTile::Subdivisions, terrainTiles[start]->subtiles[subTilePos]);
            }
        }

        terrainTiles[start]->loadedSubtiles = true;
    }

    Logger::Log("Loading region tile (", start.x, ", ", start.y, ") succeeded.");
    return true;
}

void TerrainManager::LoadHeightmapEdges(const glm::ivec2& start, int i, int j, int subSize, float* heightmap, unsigned char* types)
{
    auto yMinusOne = [&]()
    {
        if (j != 0)
        {
            // Still within the same major tile.
            return (TerrainTile::SubtileSize - 1) + (j - 1) * TerrainTile::SubtileSize;
        }
        else if (start.y != min.y)
        {
            // Still within the same set of tiles.
            return (TerrainTile::SubtileSize - 1) + (TerrainTile::Subdivisions - 1) * TerrainTile::SubtileSize;
        }
        
        return j * TerrainTile::SubtileSize;
    };

    auto yMinusOneTile = [&]()
    {
        if (j != 0)
        {
            return start.y;
        }
        else if (start.y != min.y)
        {
            return start.y - 1;
        }

        return start.y;
    };

    auto yPlusOne = [&]()
    {
        if (j != TerrainTile::Subdivisions - 1)
        {
            // Still within the same major tile.
            return (j + 1) * TerrainTile::SubtileSize;
        }
        else if (start.y != max.y)
        {
            // Still within the same set of tiles.
            return 0;
        }

        return j * TerrainTile::SubtileSize;
    };

    auto yPlusOneTile = [&]()
    {
        if (j != TerrainTile::Subdivisions - 1)
        {
            return start.y;
        }
        else if (start.y != max.y)
        {
            return start.y + 1;
        }

        return start.y;
    };

    auto xMinusOne = [&]()
    {
        if (i != 0)
        {
            // Still within the same major tile.
            return (TerrainTile::SubtileSize - 1) + (i - 1) * TerrainTile::SubtileSize;
        }
        else if (start.x != min.x)
        {
            // Still within the same set of tiles.
            return (TerrainTile::SubtileSize - 1) + (TerrainTile::Subdivisions - 1) * TerrainTile::SubtileSize;
        }

        return i * TerrainTile::SubtileSize;
    };

    auto xMinusOneTile = [&]()
    {
        if (i != 0)
        {
            return start.x;
        }
        else if (start.x != min.x)
        {
            return start.x - 1;
        }

        return start.x;
    };

    auto xPlusOne = [&]()
    {
        if (i != TerrainTile::Subdivisions - 1)
        {
            // Still within the same major tile.
            return (i + 1) * TerrainTile::SubtileSize;
        }
        else if (start.x != max.x)
        {
            // Still within the same set of tiles.
            return 0;
        }

        return i * TerrainTile::SubtileSize;
    };

    auto xPlusOneTile = [&]()
    {
        if (i != TerrainTile::Subdivisions - 1)
        {
            return start.x;
        }
        else if (start.x != max.x)
        {
            return start.x + 1;
        }

        return start.x;
    };

    // Top row.
    int yReal = yMinusOne();
    glm::ivec2 tile = glm::ivec2(start.x, yMinusOneTile());
    for (int x = 1; x < subSize - 1; x++)
    {
        int xReal = (x - 1) + i * TerrainTile::SubtileSize;
        ReadTilePixel(tile, glm::ivec2(xReal, yReal), &heightmap[x], &types[x]);
    }
    
    // Bottom row.
    yReal = yPlusOne();
    tile = glm::ivec2(start.x, yPlusOneTile());
    for (int x = 1; x < subSize - 1; x++)
    {
        int xReal = (x - 1) + i * TerrainTile::SubtileSize;
        ReadTilePixel(tile, glm::ivec2(xReal, yReal), &heightmap[x + (subSize - 1) * subSize], &types[x + (subSize - 1) * subSize]);
    }

    // Left column.
    int xReal = xMinusOne();
    tile = glm::ivec2(xMinusOneTile(), start.y);
    for (int y = 1; y < subSize - 1; y++)
    {
        int yReal = (y - 1) + j * TerrainTile::SubtileSize;
        ReadTilePixel(tile, glm::ivec2(xReal, yReal), &heightmap[y * subSize], &types[y * subSize]);
    }

    // Right column.
    xReal = xPlusOne();
    tile = glm::ivec2(xPlusOneTile(), start.y);
    for (int y = 1; y < subSize - 1; y++)
    {
        int yReal = (y - 1) + j * TerrainTile::SubtileSize;
        ReadTilePixel(tile, glm::ivec2(xReal, yReal), &heightmap[(subSize - 1) + y * subSize], &types[(subSize - 1) + y * subSize]);
    }

    // Corners
    tile = glm::ivec2(xMinusOneTile(), yMinusOneTile());
    ReadTilePixel(tile, glm::ivec2(xMinusOne(), yMinusOne()), &heightmap[0 + 0 * subSize], &types[0 + 0 * subSize]);
    
    tile = glm::ivec2(xPlusOneTile(), yPlusOneTile());
    ReadTilePixel(tile, glm::ivec2(xPlusOne(), yPlusOne()), &heightmap[(subSize - 1) + (subSize - 1) * subSize], &types[(subSize - 1) + (subSize - 1) * subSize]);

    tile = glm::ivec2(xPlusOneTile(), yMinusOneTile());
    ReadTilePixel(tile, glm::ivec2(xPlusOne(), yMinusOne()), &heightmap[(subSize - 1) + 0 * subSize], &types[(subSize - 1) + 0 * subSize]);

    tile = glm::ivec2(xMinusOneTile(), yPlusOneTile());
    ReadTilePixel(tile, glm::ivec2(xMinusOne(), yPlusOne()), &heightmap[0 + (subSize - 1) * subSize], &types[0 + (subSize - 1) * subSize]);
}

void TerrainManager::ReadTilePixel(const glm::ivec2& tile, const glm::ivec2& innerTilePos, float* heightmap, unsigned char* types)
{
    *heightmap =
        (float)((unsigned short)terrainTiles[tile]->rawImage[(innerTilePos.x + innerTilePos.y * TerrainTile::TileSize) * 4] +
            (((unsigned short)terrainTiles[tile]->rawImage[(innerTilePos.x + innerTilePos.y * TerrainTile::TileSize) * 4 + 1]) << 8))
        / (float)std::numeric_limits<unsigned short>::max();

    *types = terrainTiles[tile]->rawImage[(innerTilePos.x + innerTilePos.y * TerrainTile::TileSize) * 4 + 2];
}

bool TerrainManager::LoadTerrainTile(glm::ivec2 start, TerrainTile** tile)
{
    // The surrounding tiles must also be loaded to cache so that there are no boundary artifacts in terrain generation.
    if (!LoadTileToCache(glm::ivec2(std::min(start.x + 1, max.x), std::min(start.y + 1, max.y)), false) ||
        !LoadTileToCache(glm::ivec2(std::max(start.x - 1, min.x), std::max(start.y - 1, min.y)), false) ||
        !LoadTileToCache(glm::ivec2(std::max(start.x - 1, min.x), std::min(start.y + 1, max.y)), false) ||
        !LoadTileToCache(glm::ivec2(std::min(start.x + 1, max.x), std::max(start.y - 1, min.y)), false) ||
        !LoadTileToCache(glm::ivec2(std::min(start.x + 1, max.x), start.y), false) ||
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

    terrainEffects.Simulate(subPos + start * TerrainTile::Subdivisions, elapsedSeconds);
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
    glDrawArraysInstanced(GL_PATCHES, 0, 4, TerrainTile::SubtileSize * TerrainTile::SubtileSize);

    // Render tile SFX.
    terrainEffects.RenderSubTileEffects(subPos + start * TerrainTile::Subdivisions, perspectiveMatrix, viewMatrix, modelMatrix);
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
        terrainEffects.UnloadSubTileEffects(start * TerrainTile::Subdivisions + subTile.first);
    }

    CleanupTerrainTile(start, true);
    terrainTiles.erase(start);
}

TerrainManager::~TerrainManager()
{
    glDeleteProgram(terrainRenderProgram);

    // Cleanup any allocated terrain tiles.
    for (auto iter = terrainTiles.begin(); iter != terrainTiles.end(); iter++)
    {
        CleanupTerrainTile(iter->first, false);
    }
}

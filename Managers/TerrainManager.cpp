#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

TerrainManager::TerrainManager(ShaderManager* shaderManager, std::string terrainRootFolder, int tileSize)
    : shaderManager(shaderManager), rootFolder(terrainRootFolder), tileSize(tileSize)
{
}

int TerrainManager::GetTileSize() const
{
    return tileSize;
}

bool TerrainManager::LoadBasics()
{
    if (!shaderManager->CreateShaderProgramWithTesselation("terrainRender", &terrainRenderProgram))
    {
        Logger::LogError("Failed to load the basic terrain rendering shader; cannot continue.");
        return false;
    }

    terrainTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainTexture");
    projLocation = glGetUniformLocation(terrainRenderProgram, "projMatrix");
    mvLocation = glGetUniformLocation(terrainRenderProgram, "mvMatrix");

    return true;
}

GLuint TerrainManager::CreateHeightmapTexture(TerrainTile* tile)
{
    GLuint newTextureId;
    glGenTextures(1, &newTextureId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newTextureId);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, tileSize, tileSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tileSize, tileSize, GL_RED, GL_FLOAT, tile->heightmap);

    // Ensure we clamp to the edges to avoid border problems.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return newTextureId;
}

bool TerrainManager::LoadTerrainTile(vec::vec2i pos, TerrainTile** tile)
{
    // Load from cache if we can.
    auto iterator = terrainTiles.find(pos);
    if (iterator != terrainTiles.end())
    {
        *tile = iterator->second;
        return true;
    }

    // Else, perform a full load.
    std::stringstream tileName;
    tileName << rootFolder << "/" << pos.y << "/" << pos.x << ".png";

    TerrainTile* newTile = new TerrainTile();

    int width, height;
    if (!ImageUtils::GetRawImage(tileName.str().c_str(), &newTile->rawImage, &width, &height) || width != tileSize || height != tileSize)
    {
        Logger::Log("Failed to load tile [", pos.x, ", ", pos.y, "] because of bad image/width/height: [", width, ", ", height, ".");
        return false;
    }

    newTile->heightmap = new float[width * height];
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Combine the Red (LSB) and Green (MSB) data to get the full height range.
            newTile->heightmap[i + j * width] =
                (float)((unsigned short)newTile->rawImage[(i + j * width) * 4] + (((unsigned short)newTile->rawImage[(i + j * width) * 4 + 1]) << 8)) 
                / (float)std::numeric_limits<unsigned short>::max();
        }
    }

    newTile->heightmapTextureId = CreateHeightmapTexture(newTile);

    // After saving to OpenGL, scale accordingly.
    const float scale = 900.0f;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            newTile->heightmap[i + j * width] *= scale;
        }
    }

    terrainTiles[pos] = newTile;
    *tile = newTile;
	Logger::Log("Loaded region tile (", pos.x, ", ", pos.y, ") succeeded.");

    return true;
}

void TerrainManager::RenderTile(const vec::vec2i pos, const vec::vec2i subPos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (terrainTiles.find(pos) == terrainTiles.end())
    {
        Logger::LogWarn("Attempted to render a terrain tile not loaded with [", pos.x, ", ", pos.y, "].");
        return; 
    }

    glUseProgram(terrainRenderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->heightmapTextureId);
    glUniform1i(terrainTexLocation, 0);

    glUniformMatrix4fv(projLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(mvLocation, 1, GL_FALSE, mvMatrix);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, tileSize * tileSize);
}

void TerrainManager::CleanupTerrainTile(vec::vec2i pos)
{
	glDeleteTextures(1, &terrainTiles[pos]->heightmapTextureId);
	ImageUtils::FreeRawImage(terrainTiles[pos]->rawImage);
	delete[] terrainTiles[pos]->heightmap;
	delete terrainTiles[pos];	
}

void TerrainManager::UnloadTerrainTile(vec::vec2i pos)
{
	CleanupTerrainTile(pos);
	terrainTiles.erase(pos);
}

TerrainManager::~TerrainManager()
{
    // Cleanup any allocated terrain tiles.
    for (auto iter = terrainTiles.begin(); iter != terrainTiles.end(); iter++)
    {
		CleanupTerrainTile(iter->first);
    }
}

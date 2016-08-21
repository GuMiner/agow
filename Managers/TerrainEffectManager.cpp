#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

TerrainEffectManager::TerrainEffectManager(ShaderManager* shaderManager, int subTileSize)
    : shaderManager(shaderManager), subTileSize(subTileSize)
{
}

bool TerrainEffectManager::LoadBasics()
{
    // if (!shaderManager->CreateShaderProgramWithTesselation("terrainRender", &terrainRenderProgram))
    // {
    //     Logger::LogError("Failed to load the basic terrain rendering shader; cannot continue.");
    //     return false;
    // }
	// 
    // terrainTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainTexture");
	// terrainTypeTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainType");
    // projLocation = glGetUniformLocation(terrainRenderProgram, "projMatrix");
    // mvLocation = glGetUniformLocation(terrainRenderProgram, "mvMatrix");

    return true;
}

bool TerrainEffectManager::LoadSubTileEffects(vec::vec2i pos, SubTile* tile)
{
	if (effectData.find(pos) != effectData.end())
	{
		// Already in cache.
		return true;
	}

	effectData[pos] = new EffectData();

	return true;
}

void TerrainEffectManager::RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (effectData.find(pos) == effectData.end())
    {
        Logger::LogWarn("Attempted to render terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return; 
    }

    // glUseProgram(terrainRenderProgram);
	// 
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->subtiles[subPos]->heightmapTextureId);
    // glUniform1i(terrainTexLocation, 0);
	// 
	// glActiveTexture(GL_TEXTURE1);
	// glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->subtiles[subPos]->typeTextureId);
	// glUniform1i(terrainTypeTexLocation, 1);
	// 
    // glUniformMatrix4fv(projLocation, 1, GL_FALSE, projectionMatrix);
    // glUniformMatrix4fv(mvLocation, 1, GL_FALSE, mvMatrix);
	// 
    // glPatchParameteri(GL_PATCH_VERTICES, 4);
    // glDrawArraysInstanced(GL_PATCHES, 0, 4, GetSubTileSize() * GetSubTileSize());
}

void TerrainEffectManager::CleanupSubTileEffects(vec::vec2i pos, bool log)
{
	delete effectData[pos];

	if (log)
	{
		Logger::Log("Unloaded effects ", pos.x, ", ", pos.y, ".");
	}
}

void TerrainEffectManager::UnloadSubTileEffects(vec::vec2i pos)
{
	CleanupSubTileEffects(pos, true);
	effectData.erase(pos);
}

TerrainEffectManager::~TerrainEffectManager()
{
    // Cleanup any allocated effects 
    for (auto iter = effectData.begin(); iter != effectData.end(); iter++)
    {
		CleanupSubTileEffects(iter->first, false);
    }
}

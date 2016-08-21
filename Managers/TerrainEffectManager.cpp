#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Math\MathOps.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

TerrainEffectManager::TerrainEffectManager(ShaderManager* shaderManager, int subTileSize)
    : shaderManager(shaderManager), subTileSize(subTileSize)
{
}

bool TerrainEffectManager::LoadBasics()
{
    if (!shaderManager->CreateShaderProgram("grassRender", &grassProgram.programId))
    {
        Logger::LogError("Failed to load the basic grass rendering shader; cannot continue.");
        return false;
    }
	// 
    // terrainTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainTexture");
	// terrainTypeTexLocation = glGetUniformLocation(terrainRenderProgram, "terrainType");
    grassProgram.projMatrixLocation = glGetUniformLocation(grassProgram.programId, "projMatrix");
    grassProgram.mvMatrixLocation = glGetUniformLocation(grassProgram.programId, "mvMatrix");

    return true;
}

bool TerrainEffectManager::LoadSubTileEffects(vec::vec2i pos, SubTile* tile)
{
	if (effectData.find(pos) != effectData.end())
	{
		// Already in cache.
		return true;
	}

	EffectData* effect = new EffectData();
	effect->grassStalks.Reset();
	glGenVertexArrays(1, &effect->vao);
	glBindVertexArray(effect->vao);
	glGenBuffers(1, &effect->positionBuffer);
	glGenBuffers(1, &effect->colorBuffer);
	glGenBuffers(1, &effect->drawIdBuffer);

	// Scan the image for grass pixels.
	for (int i = 0; i < subTileSize; i++)
	{
		for (int j = 0; j < subTileSize; j++)
		{
			if (tile->type[i + j * subTileSize] == TerrainTypes::GRASSLAND)
			{
				float height = tile->heightmap[i + j * subTileSize];
				vec::vec2i realPos = pos * 0.10f + vec::vec2i(i, j);

				// TODO configurable
				vec::vec3 bottomColor = vec::vec3(0.0f, 0.90f + MathOps::Rand() * 0.10f, 0.0f);
				vec::vec3 topColor    = vec::vec3(0.0f, 0.50f + MathOps::Rand() * 0.30f, 0.20f + MathOps::Rand() * 0.60f);
				vec::vec3 bottomPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height);
				vec::vec3 topPos   = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height + 0.15f + 1.70f * MathOps::Rand());

				effect->grassStalks.positions.push_back(bottomPos);
				effect->grassStalks.positions.push_back(topPos);
				effect->grassStalks.colors.push_back(bottomColor);
				effect->grassStalks.colors.push_back(topColor);
				effect->grassStalks.ids.push_back(effect->grassStalks.positions.size() - 1); // Starts at 1
				effect->grassStalks.ids.push_back(effect->grassStalks.positions.size());
			}
		}
	}

	Logger::Log("Parsed ", effect->grassStalks.positions.size() / 2, " grass stalks.");
	effect->grassStalks.TransferPositionToOpenGl(effect->positionBuffer);
	effect->grassStalks.TransferColorToOpenGl(effect->colorBuffer);
	effect->grassStalks.TransferIdsToOpenGl(effect->drawIdBuffer);

	effectData[pos] = effect;
	return true;
}

void TerrainEffectManager::RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (effectData.find(pos) == effectData.end())
    {
        Logger::LogWarn("Attempted to render terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return; 
    }

	// Render grass
	if (effectData[pos]->grassStalks.positions.size() != 0)
	{
		glUseProgram(grassProgram.programId);
		glBindVertexArray(effectData[pos]->vao);
		// 
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->subtiles[subPos]->heightmapTextureId);
		// glUniform1i(terrainTexLocation, 0);
		// 
		// glActiveTexture(GL_TEXTURE1);
		// glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->subtiles[subPos]->typeTextureId);
		// glUniform1i(terrainTypeTexLocation, 1);
		// 

		glUniformMatrix4fv(grassProgram.projMatrixLocation, 1, GL_FALSE, projectionMatrix);
		glUniformMatrix4fv(grassProgram.mvMatrixLocation, 1, GL_FALSE, mvMatrix);

		glDrawArrays(GL_LINES, 0, effectData[pos]->grassStalks.positions.size());
	}
}

void TerrainEffectManager::CleanupSubTileEffects(vec::vec2i pos, bool log)
{
	// Cleanup grass rendering data.
	glDeleteVertexArrays(1, &effectData[pos]->vao);
	glDeleteBuffers(1, &effectData[pos]->positionBuffer);
	glDeleteBuffers(1, &effectData[pos]->colorBuffer);
	glDeleteBuffers(1, &effectData[pos]->drawIdBuffer);
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

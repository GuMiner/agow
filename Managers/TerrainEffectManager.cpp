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

	if (!shaderManager->CreateShaderProgram("roadRender", &roadProgram.programId))
	{
		Logger::LogError("Failed to load the road rendering shader; cannot continue.");
		return false;
	}

	roadProgram.projMatrixLocation = glGetUniformLocation(roadProgram.programId, "projMatrix");
	roadProgram.mvMatrixLocation = glGetUniformLocation(roadProgram.programId, "mvMatrix");

    return true;
}

void TerrainEffectManager::LoadRoadEffect(vec::vec2i pos, EffectData* effect, SubTile* tile)
{
	effect->hasRoadEffect = false;

	// Scan the image for road pixels.
	vec::vec2i lastRoadPos;
	int roadCounter = 1;
	const long ROAD_SUBCOUNT = 30;
	for (int i = 0; i < subTileSize; i++)
	{
		for (int j = 0; j < subTileSize; j++)
		{
			if (tile->type[i + j * subTileSize] == TerrainTypes::ROADS)
			{
				++roadCounter;

				if (roadCounter % ROAD_SUBCOUNT == 0)
				{
					effect->hasRoadEffect = true;
					float height = tile->heightmap[i + j * subTileSize];
					vec::vec2i realLastPos = pos * 0.10f + lastRoadPos;
					vec::vec2i realPos = pos * 0.10f + vec::vec2i(i, j);

					// TODO configurable
					vec::vec3 bottomColor = vec::vec3(0.0f, 0.50f, MathOps::Rand() * 0.20f + 0.80f);
					vec::vec3 topColor = vec::vec3(0.40f + 0.60f * MathOps::Rand(), 0.0f, 0.20f + MathOps::Rand() * 0.40f);
					vec::vec3 bottomPos = vec::vec3((float)realPos.x, (float)realPos.y, height + 2.0f);
					vec::vec3 topPos = vec::vec3((float)realLastPos.x, (float)realLastPos.y, height + 2.0f);

					// Add grass
					effect->roadEffect.travellers.positions.push_back(bottomPos);
					effect->roadEffect.travellers.positions.push_back(topPos);
					effect->roadEffect.travellers.colors.push_back(bottomColor);
					effect->roadEffect.travellers.colors.push_back(topColor);
				}

				lastRoadPos = vec::vec2i(i, j);
			}
		}
	}

	if (effect->hasRoadEffect)
	{
		glGenVertexArrays(1, &effect->roadEffect.vao);
		glBindVertexArray(effect->roadEffect.vao);
		glGenBuffers(1, &effect->roadEffect.positionBuffer);
		glGenBuffers(1, &effect->roadEffect.colorBuffer);

		Logger::Log("Parsed ", effect->roadEffect.travellers.positions.size() / 2, " road travellers.");
		effect->roadEffect.travellers.TransferPositionToOpenGl(effect->roadEffect.positionBuffer);
		effect->roadEffect.travellers.TransferColorToOpenGl(effect->roadEffect.colorBuffer);
	}
}

void TerrainEffectManager::LoadGrassEffect(vec::vec2i pos, EffectData* effect, SubTile* tile)
{
	effect->hasGrassEffect = false;

	// Scan the image for grass pixels.
	for (int i = 0; i < subTileSize; i++)
	{
		for (int j = 0; j < subTileSize; j++)
		{
			if (tile->type[i + j * subTileSize] == TerrainTypes::GRASSLAND)
			{
				effect->hasGrassEffect = true;
				float height = tile->heightmap[i + j * subTileSize];
				vec::vec2i realPos = pos * 0.10f + vec::vec2i(i, j);

				// TODO configurable
				vec::vec3 bottomColor = vec::vec3(0.0f, 0.90f + MathOps::Rand() * 0.10f, 0.0f);
				vec::vec3 topColor = vec::vec3(0.0f, 0.50f + MathOps::Rand() * 0.30f, 0.20f + MathOps::Rand() * 0.60f);
				vec::vec3 bottomPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height);
				vec::vec3 topPos = vec::vec3((float)realPos.x + 2.0f * MathOps::Rand() - 1.0f, (float)realPos.y + 2.0f * MathOps::Rand() - 1.0f, height + 0.15f + 1.70f * MathOps::Rand());

				// Add grass
				effect->grassEffect.grassStalks.positions.push_back(bottomPos);
				effect->grassEffect.grassStalks.positions.push_back(topPos);
				effect->grassEffect.grassStalks.colors.push_back(bottomColor);
				effect->grassEffect.grassStalks.colors.push_back(topColor);
				effect->grassEffect.grassStalks.ids.push_back(effect->grassEffect.grassStalks.positions.size() - 1); // Starts at 1
				effect->grassEffect.grassStalks.ids.push_back(effect->grassEffect.grassStalks.positions.size());
			}
		}
	}

	if (effect->hasGrassEffect)
	{
		glGenVertexArrays(1, &effect->grassEffect.vao);
		glBindVertexArray(effect->grassEffect.vao);
		glGenBuffers(1, &effect->grassEffect.positionBuffer);
		glGenBuffers(1, &effect->grassEffect.colorBuffer);
		glGenBuffers(1, &effect->grassEffect.drawIdBuffer);

		Logger::Log("Parsed ", effect->grassEffect.grassStalks.positions.size() / 2, " grass stalks.");
		effect->grassEffect.grassStalks.TransferPositionToOpenGl(effect->grassEffect.positionBuffer);
		effect->grassEffect.grassStalks.TransferColorToOpenGl(effect->grassEffect.colorBuffer);
		effect->grassEffect.grassStalks.TransferIdsToOpenGl(effect->grassEffect.drawIdBuffer);
	}
}

void TerrainEffectManager::UnloadGrassEffect(vec::vec2i pos)
{
	if (effectData[pos]->hasGrassEffect)
	{
		glDeleteVertexArrays(1, &effectData[pos]->grassEffect.vao);
		glDeleteBuffers(1, &effectData[pos]->grassEffect.positionBuffer);
		glDeleteBuffers(1, &effectData[pos]->grassEffect.colorBuffer);
		glDeleteBuffers(1, &effectData[pos]->grassEffect.drawIdBuffer);
	}
}

void TerrainEffectManager::UnloadRoadEffect(vec::vec2i pos)
{
	if (effectData[pos]->hasRoadEffect)
	{
		glDeleteVertexArrays(1, &effectData[pos]->roadEffect.vao);
		glDeleteBuffers(1, &effectData[pos]->roadEffect.positionBuffer);
		glDeleteBuffers(1, &effectData[pos]->roadEffect.colorBuffer);
	}
}

bool TerrainEffectManager::LoadSubTileEffects(vec::vec2i pos, SubTile* tile)
{
	if (effectData.find(pos) != effectData.end())
	{
		// Already in cache.
		return true;
	}

	EffectData* effect = new EffectData();
	LoadGrassEffect(pos, effect, tile);
	LoadRoadEffect(pos, effect, tile);

	effectData[pos] = effect;
	return true;
}

void TerrainEffectManager::Simulate(const vec::vec2i pos, float elapsedSeconds)
{
	if (effectData.find(pos) == effectData.end())
	{
		Logger::LogWarn("Attempted to simulate terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
		return;
	}

	if (effectData[pos]->hasGrassEffect)
	{
		// Modify the grass image to result in a slight waviness of the grass.
	}
}

void TerrainEffectManager::RenderSubTileEffects(const vec::vec2i pos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (effectData.find(pos) == effectData.end())
    {
        Logger::LogWarn("Attempted to render terrain effects not loaded with [", pos.x, ", ", pos.y, "].");
        return; 
    }

	// Render grass
	if (effectData[pos]->hasGrassEffect)
	{
		glUseProgram(grassProgram.programId);
		glBindVertexArray(effectData[pos]->grassEffect.vao);
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

		glDrawArrays(GL_LINES, 0, effectData[pos]->grassEffect.grassStalks.positions.size());
	}

	// Render travellers
	if (effectData[pos]->hasRoadEffect)
	{
		glUseProgram(roadProgram.programId);
		glBindVertexArray(effectData[pos]->roadEffect.vao);

		glUniformMatrix4fv(roadProgram.projMatrixLocation, 1, GL_FALSE, projectionMatrix);
		glUniformMatrix4fv(roadProgram.mvMatrixLocation, 1, GL_FALSE, mvMatrix);

		glDrawArrays(GL_LINES, 0, effectData[pos]->roadEffect.travellers.positions.size());
	}
}

void TerrainEffectManager::CleanupSubTileEffects(vec::vec2i pos, bool log)
{
	// Cleanup grass rendering data.
	UnloadGrassEffect(pos);
	UnloadRoadEffect(pos);
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

#include <iostream>
#include <limits>
#include <sstream>
#include <stb\stb_image.h>
#include "TerrainManager.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

TerrainManager::TerrainManager(vec::vec2i min, vec::vec2i max, ShaderManager* shaderManager, std::string terrainRootFolder, int tileSize)
    : min(min), max(max), shaderManager(shaderManager), rootFolder(terrainRootFolder), tileSize(tileSize), terrainEffects(shaderManager, tileSize / TerrainManager::Subdivisions)
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

bool TerrainManager::LoadTileToCache(vec::vec2i pos, bool loadSubtiles)
{
	bool loadedImage = false;
	if (terrainTiles.find(pos) != terrainTiles.end())
	{
		// Tile is already in the cache -- but if we want subtiles to be loaded and they haven't been, continue.
		loadedImage = true;
		if (terrainTiles[pos]->loadedSubtiles || terrainTiles[pos]->loadedSubtiles == loadSubtiles)
		{
			return true;
		}
	}
	else
	{
		terrainTiles[pos] = new TerrainTile();
		terrainTiles[pos]->loadedSubtiles = false;
	}

	if (!loadedImage)
	{
		std::stringstream tileName;
		tileName << rootFolder << "/" << pos.y << "/" << pos.x << ".png";

		int width, height;
		if (!ImageUtils::GetRawImage(tileName.str().c_str(), &terrainTiles[pos]->rawImage, &width, &height) || width != tileSize || height != tileSize)
		{
			Logger::Log("Failed to load tile [", pos.x, ", ", pos.y, "] because of bad image/width/height: [", width, ", ", height, ".");
			return false;
		}
	}

	if (loadSubtiles && !terrainTiles[pos]->loadedSubtiles)
	{
		int subSize = GetSubTileSize() + 1;
		for (int i = 0; i < TerrainManager::Subdivisions; i++)
		{
			for (int j = 0; j < TerrainManager::Subdivisions; j++)
			{
				float* heightmap = new float[subSize * subSize];
				for (int x = 0; x < subSize; x++)
				{
					for (int y = 0; y < subSize; y++)
					{
						// Within our main image.
						int xReal = x + i * GetSubTileSize();
						int yReal = y + j * GetSubTileSize();
						vec::vec2i tileOffset = vec::vec2i(0, 0);
						
						// If x == subSize - 1, we need to pull from the X+ terrain tile.
						// If y == subSize - 1, we need to pull from the Y+ terrain tile.
						// If both == subSize - 1, as that pixel is "unimportant", we ignore.
						if (x != subSize - 1 && y != subSize - 1)
						{
							// Do nothing, but prevet fall-through and recursive generation.
						}
						else if (x == subSize - 1 && y != subSize - 1)
						{
							if (i == TerrainManager::Subdivisions - 1)
							{
								xReal = 0;
								tileOffset = vec::vec2i(1, 0);
							}
							else
							{
								xReal++;
							}

						}
						else if (x != subSize - 1 && y == subSize - 1)
						{
							if (j == TerrainManager::Subdivisions - 1)
							{
								yReal = 0;
								tileOffset = vec::vec2i(0, 1);
							}
							else
							{
								yReal++;
							}
						}
						else
						{
							// Pull the value from x = subSize - 2, y = subSize - 2;, as we don't need it really.
							heightmap[x + y * subSize] = heightmap[(x - 1) + (y - 1) * subSize];
							continue;
						}
						
						heightmap[x + y * subSize] =
							(float)((unsigned short)terrainTiles[pos + tileOffset]->rawImage[(xReal + yReal * tileSize) * 4] +
								  (((unsigned short)terrainTiles[pos + tileOffset]->rawImage[(xReal + yReal * tileSize) * 4 + 1]) << 8))
								/ (float)std::numeric_limits<unsigned short>::max();
					}
				}

				unsigned char* types = new unsigned char[GetSubTileSize() * GetSubTileSize()];
				for (int x = 0; x < GetSubTileSize(); x++)
				{
					for (int y = 0; y < GetSubTileSize(); y++)
					{
						// Within our main image.
						int xReal = x + i * GetSubTileSize();
						int yReal = y + j * GetSubTileSize();

						types[x + y * GetSubTileSize()] = terrainTiles[pos]->rawImage[(xReal + yReal * tileSize) * 4 + 2];
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
						realHeightmap[x + y * GetSubTileSize()] = scale * heightmap[x + y * subSize];
					}
				}

				// We no longer need the data as it is now in the GPU.
				delete[] heightmap;

				vec::vec2i subTilePos = vec::vec2i(i, j);
				terrainTiles[pos]->subtiles[subTilePos] = new SubTile(heightmapTextureId, realHeightmap, typeTextureId, types);
				terrainEffects.LoadSubTileEffects(subTilePos + pos * TerrainManager::Subdivisions, terrainTiles[pos]->subtiles[subTilePos]);
			}
		}

		terrainTiles[pos]->loadedSubtiles = true;
	}

	Logger::Log("Loading region tile (", pos.x, ", ", pos.y, ") succeeded.");
	return true;
}

bool TerrainManager::LoadTerrainTile(vec::vec2i pos, TerrainTile** tile)
{
	// The +x and +y tiles must also be loaded to cache to properly generate subtile heightmaps and images.
	if (!LoadTileToCache(vec::vec2i(std::min(pos.x + 1, max.x), pos.y), false) ||
		!LoadTileToCache(vec::vec2i(pos.x, std::min(pos.y + 1, max.y)), false) ||
		!LoadTileToCache(pos, true))
	{
		return false;
	}

	*tile = terrainTiles[pos];
	return true;
}

void TerrainManager::RenderTile(const vec::vec2i pos, const vec::vec2i subPos, const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
    if (terrainTiles.find(pos) == terrainTiles.end())
    {
        Logger::LogWarn("Attempted to render a terrain tile not loaded with [", pos.x, ", ", pos.y, "].");
        return; 
    }
	else if (terrainTiles[pos]->subtiles.find(subPos) == terrainTiles[pos]->subtiles.end())
	{
		Logger::LogWarn("Attempted to render a subtile that is invalid on [", pos.x, ", ", pos.y, "], subtile [", subPos.x, ", ", subPos.y, "].");
		return;
	}

	// Render the tile.
    glUseProgram(terrainRenderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->subtiles[subPos]->heightmapTextureId);
    glUniform1i(terrainTexLocation, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrainTiles[pos]->subtiles[subPos]->typeTextureId);
	glUniform1i(terrainTypeTexLocation, 1);

    glUniformMatrix4fv(projLocation, 1, GL_FALSE, projectionMatrix);
    glUniformMatrix4fv(mvLocation, 1, GL_FALSE, mvMatrix);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, GetSubTileSize() * GetSubTileSize());

	// Render tile SFX.
	terrainEffects.RenderSubTileEffects(subPos + pos * TerrainManager::Subdivisions, projectionMatrix, mvMatrix);
}

void TerrainManager::CleanupTerrainTile(vec::vec2i pos, bool log)
{
	// We need to delete all of the subtiles and then the tile itself.
	for (std::pair<const vec::vec2i, SubTile*> subTile : terrainTiles[pos]->subtiles)
	{
		glDeleteTextures(1, &subTile.second->heightmapTextureId);
		delete[] subTile.second->heightmap;
		glDeleteTextures(1, &subTile.second->typeTextureId);
		delete[] subTile.second->type;

		delete subTile.second;
	}
	
	ImageUtils::FreeRawImage(terrainTiles[pos]->rawImage);
	delete terrainTiles[pos];

	if (log)
	{
		Logger::Log("Unloaded tile ", pos.x, ", ", pos.y, ".");
	}
}

void TerrainManager::UnloadTerrainTile(vec::vec2i pos)
{
	for (std::pair<const vec::vec2i, SubTile*> subTile : terrainTiles[pos]->subtiles)
	{
		terrainEffects.UnloadSubTileEffects(pos * TerrainManager::Subdivisions + subTile.first);
	}

	CleanupTerrainTile(pos, true);
	terrainTiles.erase(pos);
}

TerrainManager::~TerrainManager()
{
    // Cleanup any allocated terrain tiles.
    for (auto iter = terrainTiles.begin(); iter != terrainTiles.end(); iter++)
    {
		CleanupTerrainTile(iter->first, false);
    }
}

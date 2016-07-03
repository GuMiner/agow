#include <sstream>
#include <stb\stb_image.h>
#include "Terrain.h"
#include "Utils\Logger.h"
#include "Utils\ImageUtils.h"

Terrain::Terrain(ShaderManager* shaderManager, std::string terrainRootFolder, int maxTileSize, int tileWidth, int tileHeight)
    : shaderManager(shaderManager), rootFolder(terrainRootFolder), maxTileSideCount(maxTileSize), tileWidth(tileWidth), tileHeight(tileHeight)
{
}

bool Terrain::LoadBasics()
{
    // TODO -- need to load the shader series.
    return true;
}

int Terrain::GetTileId(int x, int y) const
{
    return x + y * maxTileSideCount;
}

GLuint Terrain::CreateHeightmapTexture(TerrainTile* tile)
{
    GLuint newTextureId;
    glGenTextures(1, &newTextureId);

    glBindTexture(GL_TEXTURE_2D, newTextureId);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16, tileWidth, tileHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tileWidth, tileHeight, GL_RED, GL_UNSIGNED_SHORT, tile->heightmap);

    return newTextureId;
}

bool Terrain::LoadTerrainTile(int x, int y, TerrainTile** tile)
{
    // Load from cache if we can.
    int terrainId = GetTileId(x, y);
    auto iterator = terrainTiles.find(terrainId);
    if (iterator != terrainTiles.end())
    {
        *tile = iterator->second;
        return true;
    }

    // Else, perform a full load.
    std::stringstream tileName;
    tileName << rootFolder << "/" << y << "/" << x << ".png";

    TerrainTile* newTile = new TerrainTile();

    int width, height;
    if (!ImageUtils::GetRawImage(tileName.str().c_str(), &newTile->rawImage, &width, &height) || width != tileWidth || height != tileHeight)
    {
        Logger::Log("Failed to load tile [", x, ", ", y, "] because of bad image/width/height: [", width, ", ", height, ".");
        return false;
    }

    newTile->heightmap = new unsigned short[width * height];
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Combine the Red (LSB) and Green (MSB) data to get the full height range.
            newTile->heightmap[i + j * width] = (unsigned short)newTile->rawImage[(i + j * width) * 4] + (((unsigned short)newTile->rawImage[(i + j * width) * 4 + 1]) << 8);
        }
    }

    // TODO use the other image components to store useful data. Also save out the modified raw image accordingly.
    newTile->heightmapTextureId = CreateHeightmapTexture(newTile);

    terrainTiles[terrainId] = newTile;
    *tile = newTile;
    return true;
}

void Terrain::RenderTile(int x, int y, vec::mat4& projectionMatrix, vec::mat4& mvMatrix)
{
    // TODO
}

Terrain::~Terrain()
{
    // Cleanup any allocated terrain tiles.
    for (auto iter = terrainTiles.begin(); iter != terrainTiles.end(); iter++)
    {
        glDeleteTextures(1, &iter->second->heightmapTextureId);
        stbi_image_free(iter->second->rawImage);
        delete[] iter->second->heightmap;
        delete iter->second;
    }
}

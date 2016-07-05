#include <iostream>
#include <fstream>
#include <sstream>
#include <stb\stb_image.h>
#include <stb\stb_image_resize.h>
#include <stb\stb_image_write.h>
#include "ImageUtils.h"
#include "ImageSummarizer.h"

ImageSummarizer::ImageSummarizer(int summarySize, int tileCount, int reductionFactor, std::string summaryRootPath, std::string summaryFilename)
    : summaryRootPath(summaryRootPath), summaryFilename(summaryFilename), reductionFactor(reductionFactor), summarySize(summarySize), tileId(tileCount)
{
}

void ImageSummarizer::SavePartialsFile(const char* partialsFilename)
{
    std::ofstream partialsFile(partialsFilename, std::ios::out | std::ios::binary);
    if (!partialsFile)
    {
        std::cout << "Unable to open the partials file for writing to." << std::endl;
    }

    int size = (int)missingTiles.size();
    partialsFile.write((char*)&size, sizeof(int));

    for (auto iter = missingTiles.begin(); iter != missingTiles.end(); iter++)
    {
        int id = *iter;
        partialsFile.write((char*)&id, sizeof(int));
    }

    partialsFile.close();
}

bool ImageSummarizer::TryLoadTile(int i, int j, unsigned char* summaryImage)
{
    std::stringstream imageTile;
    imageTile << "../ContourTiler/rasters/" << j << "/" << i << ".png";

    unsigned char* tileData;
    int width, height;
    if (!ImageUtils::LoadImage(imageTile.str().c_str(), &width, &height, &tileData))
    {
        std::cout << ">> Missing tile " << i << ", " << j << std::endl;
        return false;
    }
    else
    {
        // Downscale our data into floating-point data that we can then easily downscale.
        float* convertedData = new float[width * height];
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                convertedData[x + y * width] = 
                    downscaler(tileData[x + y * width + 4], tileData[x + y * width + 4] + 1, tileData[x + y * width + 4] + 2, tileData[x + y * width + 4] + 3);
            }
        }

        int subdivisions = width / reductionFactor;
        float* outputData = new float[subdivisions * subdivisions];
        if (!stbir_resize_float(convertedData, width, height, width * sizeof(float), outputData, subdivisions, subdivisions, subdivisions * sizeof(float), 1))
        {
            std::cout << "Failed to resize image: " << i << ", " << j << std::endl;
            return false;
        }
        else
        {
            // Copy the resized image over converting the floating-point data back into real-space.
            for (int x = 0; x < subdivisions; x++)
            {
                for (int y = 0; y < subdivisions; y++)
                {
                    int xReal = i * subdivisions + x;
                    int yReal = j * subdivisions + y;
                    displayConverter(outputData[x + y * subdivisions],
                        &summaryImage[(xReal + yReal * summarySize) * 4],
                        &summaryImage[(xReal + yReal * summarySize) * 4 + 1],
                        &summaryImage[(xReal + yReal * summarySize) * 4 + 2],
                        &summaryImage[(xReal + yReal * summarySize) * 4 + 3]);
                }
            }

            std::cout << "Summarized image " << i << ", " << j << std::endl;
        }

        delete[] outputData;
        delete[] convertedData;
        ImageUtils::FreeImage(tileData);
    }

    return true;
}

void ImageSummarizer::WriteSummaryImage(const char* filename, unsigned char* summaryImage)
{
    const int RGBA = 4;
    if (!stbi_write_png(filename, summarySize, summarySize, RGBA, summaryImage, summarySize * 4 * sizeof(unsigned char)))
    {
        std::cout << "Failed to write the summary file: " << stbi_failure_reason() << std::endl;
    }
}

void ImageSummarizer::CreateNewSummaryImage(const char* summaryFilename, unsigned char** summaryImage)
{
    *summaryImage = new unsigned char[summarySize * summarySize * 4];
    for (int i = 0; i < tileId.GetTileCount(); i++)
    {
        for (int j = 0; j < tileId.GetTileCount(); j++)
        {
            // Iterate through all the possible tiles, assigning them to the image (after downscaling) or the missing tile area.
            if (!TryLoadTile(i, j, *summaryImage))
            {
                missingTiles.insert(tileId.GetTileId(i, j));
            }
        }
    }

    WriteSummaryImage(summaryFilename, *summaryImage);
}

void ImageSummarizer::UpdateSummaryImage(const char* summaryFilename, unsigned char* existingImage, const char* partialsFilename)
{
    // Read in the partials file.
    std::ifstream partialsFile(partialsFilename, std::ios::in | std::ios::binary);
    if (!partialsFile)
    {
        std::cout << "Unable to open the partials file for reading from." << std::endl;
    }

    int existingItemSize;
    partialsFile.read((char*)&existingItemSize, sizeof(int));
    for (int i = 0; i < existingItemSize; i++)
    {
        int missingTile;
        partialsFile.read((char*)&missingTile, sizeof(int));

        missingTiles.insert(missingTile);
    }

    partialsFile.close();

    // For anything that's missing a tile, attempt a reload.
    bool updatedSummaryImage = false;
    std::vector<int> itemsToRemove;
    for (auto iter = missingTiles.begin(); iter != missingTiles.end(); iter++)
    {
        int x, y;
        tileId.GetPositionFromId(*iter, &x, &y);
        if (TryLoadTile(x, y, existingImage))
        {
            updatedSummaryImage = true;
            itemsToRemove.push_back(*iter);
        }
    }

    // Erase separately to avoid modifying the iterator.
    for (unsigned int i = 0; i < itemsToRemove.size(); i++)
    {
        missingTiles.erase(itemsToRemove[i]);
    }

    // Overwrite the image.
    if (updatedSummaryImage)
    {
        WriteSummaryImage(summaryFilename, existingImage);
    }
}

bool ImageSummarizer::IsTileValid(int x, int y) const
{
    return (x >= 0 && y >= 0 && x < tileId.GetTileCount() && y < tileId.GetTileCount() && missingTiles.find(tileId.GetTileId(x, y)) == missingTiles.end());
}

void ImageSummarizer::Initialize(downscaleFunction downscaler, displayConverterFunction converter)
{
    this->downscaler = downscaler;
    this->displayConverter = converter;

    std::stringstream partialsFile;
    std::stringstream summaryPath;
    summaryPath << summaryRootPath << summaryFilename;
    partialsFile << summaryRootPath << "missingImages.txt";

    // This code enables area editing while tile rendering is redone or in-progress.
    unsigned char* summaryImage;
    bool createdNewImage = false;
    int width, height;
    if (!ImageUtils::LoadImage(summaryPath.str().c_str(), &width, &height, &summaryImage))
    {
        CreateNewSummaryImage(summaryPath.str().c_str(), &summaryImage);
        createdNewImage = true;
    }
    else
    {
        UpdateSummaryImage(summaryPath.str().c_str(), summaryImage, partialsFile.str().c_str());
    }

    SavePartialsFile(partialsFile.str().c_str());

    // Send the summary image out, but flipped.
    summaryTexture.create(summarySize, summarySize);
    summaryTexture.setRepeated(false);
    summaryTexture.setSmooth(false);

    summarySprite.setTextureRect(sf::IntRect(0, height, width, -height));
    summarySprite.setTexture(summaryTexture);
    summaryTexture.update(summaryImage);

    if (createdNewImage)
    {
        delete[] summaryImage;
    }
    else
    {
        ImageUtils::FreeImage(summaryImage);
    }
}

sf::Sprite& ImageSummarizer::GetSummarizedSprite()
{
    return summarySprite;
}

void ImageSummarizer::UpdateSummaryForTile(int x, int y)
{
    // This consists of loading the summary, updating the tile in the summary, and saving it out, while updating the sprite.
    // TODO
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <stb\stb_image.h>
#include <stb\stb_image_resize.h>
#include <stb\stb_image_write.h>
#include "ImageUtils.h"
#include "ImageSummarizer.h"

ImageSummarizer::ImageSummarizer(int summarySize, int tileSideCount, int offsetX, int offsetY, int visibleSideCount, int reductionFactor, std::string summaryRootPath, std::string summaryFilename)
    : summarySize(summarySize), tileId(tileSideCount), reductionFactor(reductionFactor), tileImageSize(1000),
      summaryRootPath(summaryRootPath), summaryFilename(summaryFilename), offsetX(offsetX), offsetY(offsetY), visibleSideCount(visibleSideCount)
{
}

bool ImageSummarizer::TryLoadTile(bool verbose, int i, int j, unsigned char* summaryImage)
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
                    downscaler(tileData[(x + y * width) * 4], tileData[(x + y * width) * 4 + 1], tileData[(x + y * width) * 4 + 2], tileData[(x + y * width) * 4 + 3]);
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
                    int xReal = (i - offsetX) * subdivisions + x;
                    int yReal = (j - offsetY) * subdivisions + y;
                    displayConverter(outputData[x + y * subdivisions],
                        &summaryImage[(xReal + yReal * summarySize) * 4],
                        &summaryImage[(xReal + yReal * summarySize) * 4 + 1],
                        &summaryImage[(xReal + yReal * summarySize) * 4 + 2],
                        &summaryImage[(xReal + yReal * summarySize) * 4 + 3]);
                }
            }

            if (verbose)
            {
                std::cout << "Summarized image " << i << ", " << j << std::endl;
            }
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
    for (int i = offsetX; i < visibleSideCount; i++)
    {
        for (int j = offsetY; j < visibleSideCount; j++)
        {
            // Iterate through all the possible tiles, assigning them to the image (after downscaling) or the missing tile area.
            if (!TryLoadTile(true, i, j, *summaryImage))
            {
                std::cout << "WARNING: Missing tile " << i << ", " << j << std::endl;
            }
        }
    }

    WriteSummaryImage(summaryFilename, *summaryImage);
}

bool ImageSummarizer::IsTileValid(int x, int y) const
{
    // Restricted to the valid road area plus a bit more in the y direction.
    return (x >= 5 && y >= 16 && x <= 40 && y <= 54);
}

void ImageSummarizer::Initialize(downscaleFunction downscaler, displayConverterFunction converter)
{
    this->downscaler = downscaler;
    this->displayConverter = converter;

    std::stringstream summaryPath;
    summaryPath << summaryRootPath << summaryFilename;

    // This code enables area editing while tile rendering is redone or in-progress.
    unsigned char* summaryImage;
    bool createdNewImage = false;
    int width, height;
    if (!ImageUtils::LoadImage(summaryPath.str().c_str(), &width, &height, &summaryImage))
    {
        CreateNewSummaryImage(summaryPath.str().c_str(), &summaryImage);
        createdNewImage = true;
    }

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

// Updates the data for the specified tile, which also updates the summary.
// If saveEnabled is false, this just reloads the specified tile from memory.
void ImageSummarizer::UpdateSummaryForTile(unsigned char* newData, int x, int y, bool saveEnabled)
{
    // If enabled, save our updated tile.
    if (saveEnabled)
    {
        std::stringstream imageTile;
        imageTile << "../ContourTiler/rasters/" << y << "/" << x << ".png";

        const int RGBA = 4;
        if (!stbi_write_png(imageTile.str().c_str(), tileImageSize, tileImageSize, RGBA, newData, tileImageSize * 4 * sizeof(unsigned char)))
        {
            std::cout << "Failed to write the updated tile: " << stbi_failure_reason() << std::endl;
        }
    }

    std::stringstream summaryPath;
    summaryPath << summaryRootPath << summaryFilename;
    
    // This code enables area editing while tile rendering is redone or in-progress.
    unsigned char* summaryImage;
    int width, height;
    if (ImageUtils::LoadImage(summaryPath.str().c_str(), &width, &height, &summaryImage) && TryLoadTile(false, x, y, summaryImage))
    {
        std::cout << "Updating summary image for tile " << x << ", " << y << std::endl;
        
        // Save the updated summary image (as it may be modified).
        WriteSummaryImage(summaryPath.str().c_str(), summaryImage);
        
        // Update the sprite with our summary updates.
        summaryTexture.update(summaryImage);
        ImageUtils::FreeImage(summaryImage);
    }
}

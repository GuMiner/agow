#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <stb/stb_image_write.h>
#include "ImageUtils.h"
#include "SummaryView.h"

SummaryView::SummaryView(int size, int tileCount, int reductionFactor)
    : isAlive(true), tileCount(tileCount), size(size), reductionFactor(reductionFactor), summaryRootPath("../ContourTiler/rasters/summary/")
{

}

bool SummaryView::TryLoadTile(int i, int j, unsigned char* summaryImage)
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
        float* convertedData = new float[width * height];
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                convertedData[x + y * width] =
                    ((float)((unsigned short)tileData[(x + y * width) * 4] +
                        (((unsigned short)tileData[(x + y * width) * 4 + 1]) << 8))) / (float)std::numeric_limits<unsigned short>::max();
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
            // Copy the resized image over.
            for (int x = 0; x < subdivisions; x++)
            {
                for (int y = 0; y < subdivisions; y++)
                {
                    unsigned short averageValue = (unsigned short)(outputData[x + y * subdivisions] * (float)std::numeric_limits<unsigned short>::max());

                    int xReal = i * subdivisions + x;
                    int yReal = j * subdivisions + y;
                    summaryImage[(xReal + yReal * size) * 4] = (unsigned char)(averageValue / 256);
                    summaryImage[(xReal + yReal * size) * 4 + 1] = (unsigned char)(averageValue / 256);
                    summaryImage[(xReal + yReal * size) * 4 + 2] = (unsigned char)(averageValue / 256);
                    summaryImage[(xReal + yReal * size) * 4 + 3] = 255;
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

void SummaryView::CreateNewSummaryImage(const char* summaryFilename, unsigned char** summaryImage)
{
    *summaryImage = new unsigned char[size * size * 4];
    for (int i = 0; i < tileCount; i++)
    {
        for (int j = 0; j < tileCount; j++)
        {
            // Iterate through all the possible tiles, assigning them to the image (after downscaling) or the missing tile area.
            if (!TryLoadTile(i, j, *summaryImage))
            {
                missingTiles.insert(GetTileId(i, j));
            }
        }
    }

    // Save our summary image.
    const int RGBA = 4;
    if (!stbi_write_png(summaryFilename, size, size, RGBA, (*summaryImage), size * 4 * sizeof(unsigned char)))
    {
        std::cout << "Failed to write the summary file: " << stbi_failure_reason() << std::endl;
    }
}

void SummaryView::UpdateSummaryImage(const char* summaryFilename, unsigned char* existingImage, const char* partialsFilename)
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
        GetPositionFromId(*iter, &x, &y);
        if (TryLoadTile(x, y, existingImage))
        {
            updatedSummaryImage = true;
            itemsToRemove.push_back(*iter);
        }
    }

    // Erase separately to avoid modifying the iterator.
    for (int i = 0; i < itemsToRemove.size(); i++)
    {
        missingTiles.erase(itemsToRemove[i]);
    }

    // Overwrite the image.
    if (updatedSummaryImage)
    {
        const int RGBA = 4;
        if (!stbi_write_png(summaryFilename, size, size, RGBA, &existingImage[0], size * 4 * sizeof(unsigned char)))
        {
            std::cout << "Failed to write the summary file: " << stbi_failure_reason() << std::endl;
        }
    }
}

void SummaryView::SavePartialsFile(const char* partialsFilename)
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

void SummaryView::LoadOrUpdateSummaryView()
{
    std::stringstream partialsFile;
    std::stringstream summaryPath;
    summaryPath << summaryRootPath << "summary.png";
    partialsFile << summaryRootPath << "missingImages.txt";

    // This code enables area editing while tile rendering is redone or in-progress.
    unsigned char* summaryImage;
    int width, height;
    if (!ImageUtils::LoadImage(summaryPath.str().c_str(), &width, &height, &summaryImage))
    {
        CreateNewSummaryImage(summaryPath.str().c_str(), &summaryImage);
    }
    else
    {
        UpdateSummaryImage(summaryPath.str().c_str(), summaryImage, partialsFile.str().c_str());
    }

    SavePartialsFile(partialsFile.str().c_str());
    
    // Send the summary image out, but flipped.
    summaryTexture.create(size, size);
    summaryTexture.setRepeated(false);
    summaryTexture.setSmooth(false);

    summarySprite.scale(sf::Vector2f(1.0f, -1.0f));
    summarySprite.setTexture(summaryTexture);

    // Copy over to the image
    sf::Uint8* pixels = new sf::Uint8[size * size * 4]; // * 4 because pixels have 4 components (RGBA)
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            pixels[(i + j * size) * 4] = summaryImage[(i + j * size) * 4];
            pixels[(i + j * size) * 4 + 1] = summaryImage[(i + j * size) * 4 + 1];
            pixels[(i + j * size) * 4 + 2] = summaryImage[(i + j * size) * 4 + 2];
            pixels[(i + j * size) * 4 + 3] = summaryImage[(i + j * size) * 4 + 3];
        }
    }

    summaryTexture.update(pixels);
    delete[] pixels;
}

int SummaryView::GetTileId(int x, int y) const
{
    return x + tileCount * y;
}

void SummaryView::GetPositionFromId(int pos, int* x, int* y) const
{
    *x = pos % tileCount;
    *y = pos / tileCount;
}

bool SummaryView::IsTileValid(int x, int y) const
{
    return (x >= 0 && y >= 0 && x < tileCount && y < tileCount && missingTiles.find(GetTileId(x, y)) == missingTiles.end());
}

void SummaryView::HandleEvents(sf::RenderWindow& window)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            // Ignore -- we only close if the main map editor closes.
        }
    }
}

void SummaryView::Render(sf::RenderWindow& window)
{
    window.draw(summarySprite);
}

void SummaryView::ThreadStart()
{
    LoadOrUpdateSummaryView();

    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style = sf::Style::Titlebar;
    sf::RenderWindow window(sf::VideoMode(size, size), "Summary", style, contextSettings);
    window.setFramerateLimit(60);

    // Start the main loop
    while (isAlive)
    {
        HandleEvents(window);
        Render(window);

        // Display what we rendered.
        window.display();
    }
}

void SummaryView::Start()
{
    executionThread = new std::thread(&SummaryView::ThreadStart, this);
}

void SummaryView::Stop()
{
    isAlive = false;
    executionThread->join();
    delete executionThread;
}
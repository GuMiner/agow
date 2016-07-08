#pragma once
#include <functional>
#include <set>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include "TileIdentifier.h"

// Handles the aggregation of multiple images into the overall summary image.
typedef std::function<float(unsigned char, unsigned char, unsigned char, unsigned char)> downscaleFunction;
typedef std::function<void(float, unsigned char*, unsigned char*, unsigned char*, unsigned char*)> displayConverterFunction;

class ImageSummarizer
{
    int summarySize;
    int reductionFactor;
    int tileImageSize;
    std::string summaryRootPath;
    std::string summaryFilename;

    TileIdentifier tileId;
    
    downscaleFunction downscaler;
    displayConverterFunction displayConverter;
    
    // Loads a tile into the provided summary image, using the appropriate converter functions.
    bool TryLoadTile(bool verbose, int x, int y, unsigned char* summaryImage);

    // Writes out the full summary image.
    void WriteSummaryImage(const char* filename, unsigned char* summaryImage);

    unsigned char* rawSummary;
    sf::Sprite summarySprite;
    sf::Texture summaryTexture;

    void CreateNewSummaryImage(const char* summaryFilename, unsigned char** summaryImage);

public:
    ImageSummarizer(int summarySize, int tileCount, int reductionFactor, std::string summaryRootPath, std::string summaryFilename);
    void Initialize(downscaleFunction downscaler, displayConverterFunction converter);

    bool IsTileValid(int x, int y) const;
    sf::Sprite& GetSummarizedSprite();

    // Updates the data for the specified tile, which also updates the summary.
    // If saveEnabled is false, this just reloads the specified tile from memory.
    void UpdateSummaryForTile(unsigned char* newData, int x, int y, bool saveEnabled);
};


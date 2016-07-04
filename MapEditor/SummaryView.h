#pragma once
#include <set>
#include <thread>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>

class SummaryView
{
    int size;
    int tileCount;

    // Amount we reduce the main images by (when generating a summary) given our screen size.
    int reductionFactor;

    std::string summaryRootPath;

    int GetTileId(int x, int y) const;
    void GetPositionFromId(int pos, int* x, int* y) const;

    std::set<int> missingTiles;
    void SavePartialsFile(const char* partialsFilename);

    sf::Sprite summarySprite;
    sf::Texture summaryTexture;
    bool TryLoadTile(int x, int y, unsigned char* summaryImage);
    void CreateNewSummaryImage(const char* summaryFilename, unsigned char** summaryImage);
    void UpdateSummaryImage(const char* summaryFilename, unsigned char* existingImage, const char* partialsFilename);
    void LoadOrUpdateSummaryView();
    
    void HandleEvents(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window);

    bool isAlive;
    std::thread* executionThread;
    void ThreadStart();

    sf::RectangleShape selectedTileRectangle;
    int selectedTile;
    void UpdateSelectedTileRectangle();
public:
    enum Direction
    {
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    void MoveSelectedTile(Direction direction);
    void LoadSelectedTile(unsigned char** rawData);

    SummaryView(int size, int tileCount, int reductionFactor);
    void Start();
    bool IsTileValid(int x, int y) const;
    void Stop();
};


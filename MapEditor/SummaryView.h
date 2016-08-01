#pragma once
#include <set>
#include <thread>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include "ImageSummarizer.h"
#include "TileIdentifier.h"

class SummaryView
{
    int offsetX;
    int offsetY;
    int visibleSideCount;

    int size;
    
    TileIdentifier tileId;
    ImageSummarizer topographicSummarizer;
    ImageSummarizer overlaySummarizer;

    void HandleEvents(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window);

    bool isAlive;
    std::thread* executionThread;
    void ThreadStart();

    sf::RectangleShape selectedTileRectangle;
    int selectedTile;
    void UpdateSelectedTileRectangle();

    void SummaryView::LoadSelectedTile(unsigned char** data, int offsetX, int offsetY);

    sf::RenderWindow* windowSelf;

public:
    enum Direction
    {
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    void RemapToTile(double* x, double* y);

    void MoveSelectedTile(Direction direction);
    void LoadSelectedTile(bool loadEdges, unsigned char** centerData, unsigned char** leftData, unsigned char** rightData, unsigned char** topData, unsigned char** bottomData);
    void UpdateSelectedTile(unsigned char* newData);

    SummaryView(int size, int tileCount, int offsetX, int offsetY, int visibleSideCount, int reductionFactor);
    void Start();
    void Stop();
};


#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include "PaletteWindow.h"
#include "SummaryView.h"

// Allows for large-scale editing of the map tiles generated from CountourTiler
class MapEditor
{
    void LoadGraphics();
    void HandleEvents(sf::RenderWindow& window, bool& alive);
    void Render(sf::RenderWindow& window);

    // Total display size of the window.
    int size;

    // Max tiles in x-y direction.
    int tileCount;

    // Size of a tile.
    int tileSize;

    SummaryView summaryView;
    PaletteWindow paletteWindow;

    unsigned char* rawTileData;
    sf::Sprite currentTile;
    sf::Texture currentTileTexture;
    void UpdateCurrentTileTexture();

public:
    MapEditor();

    // Runs the game loop.
    void Run();
};
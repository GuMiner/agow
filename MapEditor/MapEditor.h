#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include "PaletteWindow.h"
#include "SummaryView.h"

struct DisplaySettings
{
    bool rescale;
    bool showOverlay;
    bool showContours;

    DisplaySettings()
    {
        rescale = false;
        showOverlay = true;
        showContours = false;
    }
};

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
    DisplaySettings displaySettings;

    bool mouseDown;

    // Updates the current tile with the given rawTileData. Performs appropriate conversions.
    void UpdateCurrentTile();

    unsigned char* rawTileData;
    sf::Sprite currentTile;
    sf::Texture currentTileTexture;

    // Replaces the current on-screen tile.
    void UpdateCurrentTileTexture();

public:
    MapEditor();

    // Runs the game loop.
    void Run();
};
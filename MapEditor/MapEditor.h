#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include "PaletteWindow.h"
#include "RoadFeatures.h"
#include "SummaryView.h"

struct DisplaySettings
{
    bool rescale;
    bool showOverlay;
    bool showContours;
    bool showHeightmap;
    
    DisplaySettings()
    {
        rescale = true;
        showOverlay = true;
        showContours = true;
        showHeightmap = true;
    }
};

struct Brushes
{
    sf::RectangleShape squareBrush;
    sf::CircleShape circleBrush;

    Brushes(float startingRadius)
        : squareBrush(sf::Vector2f(startingRadius * 2.0f, startingRadius * 2.0f)), circleBrush(startingRadius, 32)
    {
    }
};

struct CurrentTile
{
    unsigned short minHeight;
    unsigned short maxHeight;

    unsigned char* center;
    unsigned char* left;
    unsigned char* right;
    unsigned char* up;
    unsigned char* down;
    
    sf::Sprite centerSprite;
    sf::Texture centerTexture;

    sf::Sprite leftSprite;
    sf::Texture leftTexture;
    
    sf::Sprite rightSprite;
    sf::Texture rightTexture;

    sf::Sprite upSprite;
    sf::Texture upTexture;
    
    sf::Sprite downSprite;
    sf::Texture downTexture;

    CurrentTile()
        : center(), left(), right(), up(), down()
    {
    }
};

struct Limits
{
    int minX;
    int maxX;
    int minY;
    int maxY;
};

// Allows for large-scale editing of the map tiles generated from CountourTiler
class MapEditor
{
    void OutputHelp();
    void LoadGraphics();
    void HandleEvents(sf::RenderWindow& window, bool& alive);
    void Render(sf::RenderWindow& window);

    // Total display size of the window.
    int size;

    // Offset to show four corners to align drawing.
    int offset;

    // Max tiles in x-y direction.
    int tileCount;

    // Size of a tile.
    int tileSize;

    RoadFeatures roadFeatures;
    SummaryView summaryView;
    PaletteWindow paletteWindow;
    DisplaySettings displaySettings;
    Brushes brushes;
    
    bool mouseDown;
    sf::Uint8* convertedRawData;

    // Applies the drawing at the current mouse position, following the current overwrite settings.
    void RoadDraw(bool redrawArea);
    void BarricadeDraw(bool redrawArea);
    void EmitterDraw(bool redrawArea);
    void StopDraw(bool redrawArea);

    void EraseTile();
    void DrawWithoutRescaleOrRedraw(PaletteWindow::Tool tool, float radius, unsigned char terrainId, int mouseX, int mouseY, Limits* limits);
    void DrawWithoutRedraw(PaletteWindow::Tool tool, float radius, unsigned char terrainId, int mouseX, int mouseY, Limits* limits);
    void Draw(PaletteWindow::Tool tool, float radius, unsigned char terrainId, int mouseX, int mouseY);

    CurrentTile currentTile;
    void CreateSpriteTexturePair(sf::Sprite& sprite, sf::Texture& texture, sf::Vector2f spritePos, sf::IntRect textureRect);

    // Redraws the entire current tile.
    void RedrawCurrentTiles();
    void UpdateMinMaxHeights();
    void RedrawSelectedArea(unsigned char* data, sf::Texture& texture, int xMin, int xMax, int yMin, int yMax);

    bool saveOnMove;
    void SaveTile();

public:
    MapEditor();

    // Runs the game loop.
    void Run();
};
#pragma once
#include <thread>
#include <map>
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>

// Shows the terrain type palette and tool selection.
class PaletteWindow
{
public:
    enum TerrainType
    {
        SNOW_PEAK,
        ROCKS,
        TREES,
        DIRTLAND,
        GRASSLAND,
        ROADS,
        CITY,
        SAND,
        RIVER,
        LAKE,
        COUNT
    };

    enum Tool
    {
        ERASE,
        SQUARE_BRUSH,
        CIRCLE_BRUSH
    };

private:
    Tool selectedTool;
    float toolRadius; // In actual-image pixels.

    TerrainType selectedTerrain;

    sf::Font typeFont;
    std::map<TerrainType, sf::Text> terrainText;
    std::map<TerrainType, sf::RectangleShape> terrainRectangles;

    int size;

    void HandleEvents(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window);

    void LoadGraphics();

    bool isAlive;
    std::thread* executionThread;
    void ThreadStart();

public:
    PaletteWindow(int size);
    void Start();
    
    sf::Color GetTerrainColor(TerrainType type) const;
    std::string GetTerrainName(TerrainType type) const;
    Tool GetSelectedTool() const;
    float GetToolRadius() const;
    TerrainType GetTerrainType() const;

    void Stop();
};


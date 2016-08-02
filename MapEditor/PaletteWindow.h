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
        CIRCLE_BRUSH,
        COUNT_TOOL
    };

private:
    Tool selectedTool;
    int toolRadius; // In actual-image pixels.
    std::string GetToolSizeText() const;

    TerrainType selectedTerrain;

    sf::Font typeFont;

    float terrainSize;
    float terrainOffset;
    float terrainXOffset;
    
    std::map<TerrainType, sf::Text> terrainText;
    std::map<TerrainType, sf::RectangleShape> terrainRectangles;

    float toolSize;
    float toolOffset;
    float toolXOffset;
    sf::Text toolSizeText;
    std::map<Tool, sf::Text> toolText;
    std::map<Tool, sf::RectangleShape> toolRectangles;

    // Display size
    int size;

    void HandleEvents(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window);

    void LoadGraphics();

    bool isAlive;
    std::thread* executionThread;
    void ThreadStart();

public:
    PaletteWindow(int size);
    
    // TODO should be getter/setter protected.
    bool allowOverwrite;

    void Start();

    void HandleKeyEvent(sf::Event& event);
    
    static sf::Color GetTerrainColor(TerrainType type);
    static float GetHue(sf::Color color); // [0 to 1]

    static std::string GetTerrainName(TerrainType type);
    static std::string GetToolName(Tool type);
    static unsigned char GetTerrainId(TerrainType type);
    static TerrainType GetNearestTerrainType(unsigned char value);

    Tool GetSelectedTool() const;
    int GetToolRadius() const;
    TerrainType GetTerrainType() const;

    void Stop();
};


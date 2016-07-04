#include <iostream>
#include "PaletteWindow.h"

PaletteWindow::PaletteWindow(int size)
    : isAlive(true), size(size), selectedTool(Tool::SQUARE_BRUSH), toolRadius(10.0f), selectedTerrain(TerrainType::TREES)
{
}

void PaletteWindow::LoadGraphics()
{
    if (!typeFont.loadFromFile("../fonts/DejaVuSans.ttf"))
    {
        std::cout << "Font loading unsuccessful!" << std::endl;
    }

    float size = 40.0f;
    float offset = 10.0f;
    for (int i = 0; i < (int)TerrainType::COUNT; i++)
    {
        terrainRectangles[(TerrainType)i] = sf::RectangleShape(sf::Vector2f(size, size));
        terrainRectangles[(TerrainType)i].setFillColor(GetTerrainColor((TerrainType)i));
        terrainRectangles[(TerrainType)i].setPosition(sf::Vector2f(20.f, offset + i * (size + offset)));

        terrainText[(TerrainType)i] = sf::Text(GetTerrainName((TerrainType)i), typeFont, 20);
        terrainText[(TerrainType)i].setColor(GetTerrainColor((TerrainType)i));
        terrainText[(TerrainType)i].setPosition(sf::Vector2f(20.0f + size + offset, offset + i * (size + offset)));
    }

    terrainRectangles[selectedTerrain].setOutlineColor(sf::Color::Cyan);
    terrainRectangles[selectedTerrain].setOutlineThickness(1.0f);
}

void PaletteWindow::HandleEvents(sf::RenderWindow& window)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            // Ignore -- we only close if the main map editor closes.
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            // Selecting a new terrain item.
        }
    }
}

void PaletteWindow::Render(sf::RenderWindow& window)
{
    for (auto iter = terrainRectangles.begin(); iter != terrainRectangles.end(); iter++)
    {
        window.draw(iter->second);
    }
    
    for (auto iter = terrainText.begin(); iter != terrainText.end(); iter++)
    {
        window.draw(iter->second);
    }
}

sf::Color PaletteWindow::GetTerrainColor(PaletteWindow::TerrainType type) const
{
    switch (type)
    {
    case SNOW_PEAK: return sf::Color(245, 244, 253);
    case ROCKS: return sf::Color(140, 115, 115);
    case TREES: return sf::Color(22, 137, 10);
    case DIRTLAND: return sf::Color(185, 122, 87);
    case GRASSLAND: return sf::Color(122, 243, 129);
    case ROADS: return sf::Color(192, 192, 192);
    case CITY: return sf::Color(170, 34, 181);
    case SAND: return sf::Color(255, 255, 128);
    case RIVER: return sf::Color(121, 121, 255);
    case LAKE: return sf::Color(0, 0, 179);
    default: return sf::Color(0, 255, 255);
    }
}

std::string PaletteWindow::GetTerrainName(TerrainType type) const
{
    switch (type)
    {
    case SNOW_PEAK: return "Snow Peak";
    case ROCKS: return "Rocks";
    case TREES: return "Trees";
    case DIRTLAND: return "Dirtland";
    case GRASSLAND: return "Grassland";
    case ROADS: return "Roads";
    case CITY: return "City";
    case SAND: return "Sand";
    case RIVER: return "River";
    case LAKE: return "Lake";
    default: return "Unknown!";
    }
}

PaletteWindow::Tool PaletteWindow::GetSelectedTool() const
{
    return selectedTool;
}

float PaletteWindow::GetToolRadius() const
{
    return toolRadius;
}

PaletteWindow::TerrainType PaletteWindow::GetTerrainType() const
{
    return selectedTerrain;
}

void PaletteWindow::ThreadStart()
{
    LoadGraphics();

    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style = sf::Style::Titlebar;
    sf::RenderWindow window(sf::VideoMode(size, size * 3), "Palette", style, contextSettings);
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

void PaletteWindow::Start()
{
    executionThread = new std::thread(&PaletteWindow::ThreadStart, this);
}

void PaletteWindow::Stop()
{
    isAlive = false;
    executionThread->join();
    delete executionThread;
}
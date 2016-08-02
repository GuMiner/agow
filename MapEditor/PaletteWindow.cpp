#include <iostream>
#include <sstream>
#include "PaletteWindow.h"

PaletteWindow::PaletteWindow(int size)
    : isAlive(false), size(size), selectedTool(Tool::SQUARE_BRUSH), toolRadius(10), selectedTerrain(TerrainType::TREES),
      terrainSize(40.f), terrainOffset(10.0f), terrainXOffset(20.0f), toolSize(15.0f), toolOffset(5.0f), toolXOffset(10.0f), allowOverwrite(false)
{
}

std::string PaletteWindow::GetToolSizeText() const
{
    std::stringstream toolSizeStream;
    toolSizeStream << "Tool Radius: " << toolRadius << std::endl;
    return toolSizeStream.str();
}

void PaletteWindow::LoadGraphics()
{
    if (!typeFont.loadFromFile("../fonts/DejaVuSans.ttf"))
    {
        std::cout << "Font loading unsuccessful!" << std::endl;
    }

    for (int i = 0; i < (int)TerrainType::COUNT; i++)
    {
        terrainRectangles[(TerrainType)i] = sf::RectangleShape(sf::Vector2f(terrainSize, terrainSize));
        terrainRectangles[(TerrainType)i].setFillColor(GetTerrainColor((TerrainType)i));
        terrainRectangles[(TerrainType)i].setPosition(sf::Vector2f(terrainXOffset, terrainOffset + i * (terrainSize + terrainOffset)));
        terrainRectangles[(TerrainType)i].setOutlineColor(sf::Color::Cyan);

        terrainText[(TerrainType)i] = sf::Text(GetTerrainName((TerrainType)i), typeFont, 18);
        terrainText[(TerrainType)i].setColor(GetTerrainColor((TerrainType)i));
        terrainText[(TerrainType)i].setPosition(sf::Vector2f(terrainXOffset + terrainSize + terrainOffset, terrainOffset + i * (terrainSize + terrainOffset)));
    }

    terrainRectangles[selectedTerrain].setOutlineThickness(1.0f);

    for (int i = 0; i < (int)Tool::COUNT_TOOL; i++)
    {
        toolRectangles[(Tool)i] = sf::RectangleShape(sf::Vector2f(toolSize, toolSize));
        toolRectangles[(Tool)i].setFillColor(sf::Color::Yellow);
        toolRectangles[(Tool)i].setPosition(sf::Vector2f(toolXOffset,
            toolOffset + i * (toolOffset + toolSize) + (int)TerrainType::COUNT * (terrainSize + terrainOffset)));
        toolRectangles[(Tool)i].setOutlineColor(sf::Color::Cyan);

        toolText[(Tool)i] = sf::Text(GetToolName((Tool)i), typeFont, 15);
        toolText[(Tool)i].setColor(sf::Color::Green);
        toolText[(Tool)i].setPosition(toolRectangles[(Tool)i].getPosition() + sf::Vector2f(toolSize + toolOffset, 0.0f));
    }

    toolRectangles[selectedTool].setOutlineThickness(1.0f);

    toolSizeText = sf::Text(GetToolSizeText(), typeFont, 16);
    toolSizeText.setColor(sf::Color::White);
    toolSizeText.setPosition(toolRectangles[(Tool)(Tool::COUNT_TOOL - 1)].getPosition() + sf::Vector2f(0.0f, toolSize + toolOffset));
}

void PaletteWindow::HandleKeyEvent(sf::Event& event)
{
    switch (event.key.code)
    {
    case sf::Keyboard::Add: toolRadius++; toolSizeText.setString(GetToolSizeText()); break;
    case sf::Keyboard::Subtract: toolRadius--; if (toolRadius < 1) { toolRadius = 1; } toolSizeText.setString(GetToolSizeText()); break;
    case sf::Keyboard::W: allowOverwrite = !allowOverwrite; std::cout << "Overwrite status: " << allowOverwrite << std::endl; break;
    default: break;
    }
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
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            // Selecting something, figure out which.
            bool swappedTerrain = false;
            for (int i = 0; i < (int)TerrainType::COUNT; i++)
            {
                if (terrainRectangles[(TerrainType)i].getGlobalBounds().contains(sf::Vector2f((float)event.mouseButton.x, (float)event.mouseButton.y)))
                {
                    std::cout << "Switching to terrain type " << GetTerrainName((TerrainType)i) << std::endl;
                    terrainRectangles[selectedTerrain].setOutlineThickness(0);
                    terrainRectangles[(TerrainType)i].setOutlineThickness(1.0f);

                    selectedTerrain = (TerrainType)i;
                    swappedTerrain = true;
                    break;
                }
            }

            if (!swappedTerrain)
            {
                // Determine if we instead swapped tools.
                for (int i = 0; i < (int)Tool::COUNT_TOOL; i++)
                {
                    if (toolRectangles[(Tool)i].getGlobalBounds().contains(sf::Vector2f((float)event.mouseButton.x, (float)event.mouseButton.y)))
                    {
                        std::cout << "Switching to tool " << GetToolName((Tool)i) << std::endl;

                        toolRectangles[selectedTool].setOutlineThickness(0);
                        toolRectangles[(Tool)i].setOutlineThickness(1.0f);

                        selectedTool = (Tool)i;
                        break;
                    }
                }
            }
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            HandleKeyEvent(event);
        }
    }
}

void PaletteWindow::Render(sf::RenderWindow& window)
{
    window.clear();

    for (auto iter = terrainRectangles.begin(); iter != terrainRectangles.end(); iter++)
    {
        window.draw(iter->second);
    }
    
    for (auto iter = terrainText.begin(); iter != terrainText.end(); iter++)
    {
        window.draw(iter->second);
    }

    for (auto iter = toolRectangles.begin(); iter != toolRectangles.end(); iter++)
    {
        window.draw(iter->second);
    }

    for (auto iter = toolText.begin(); iter != toolText.end(); iter++)
    {
        window.draw(iter->second);
    }

    window.draw(toolSizeText);
}

sf::Color PaletteWindow::GetTerrainColor(TerrainType type)
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

float PaletteWindow::GetHue(sf::Color color)
{
    float r = (float)color.r / 255;
    float g = (float)color.g / 255;
    float b = (float)color.b / 255;

    float min = std::min(std::min(r, g), b);
    float max = std::max(std::max(r, g), b);

    float delta = max - min;

    float middle = 0.00001f;
    float hue;
    if (std::abs(r - max) < middle)
    {
        hue = (g - b) / delta;
    }
    else if (std::abs(g - max) < middle)
    {
        hue = 2 + (b - r) / delta;
    }
    else
    {
        hue = 4 + (r - g) / delta;
    }

    if (hue < 1)
    {
        hue += 1;
    }

    return hue;
}

unsigned char PaletteWindow::GetTerrainId(TerrainType type)
{
    switch (type)
    {
    case SNOW_PEAK: return 0;
    case ROCKS: return 25;
    case TREES: return 50;
    case DIRTLAND: return 75;
    case GRASSLAND: return 100;
    case ROADS: return 125;
    case CITY: return 150;
    case SAND: return 175;
    case RIVER: return 200;
    case LAKE: return 225;
    default: return 255;
    }
}

PaletteWindow::TerrainType PaletteWindow::GetNearestTerrainType(unsigned char value)
{
    if (value < GetTerrainId(LAKE))
    {
        if (value < GetTerrainId(RIVER))
        {
            if (value < GetTerrainId(SAND))
            {
                if (value < GetTerrainId(CITY))
                {
                    if (value < GetTerrainId(ROADS))
                    {
                        if (value < GetTerrainId(GRASSLAND))
                        {
                            if (value < GetTerrainId(DIRTLAND))
                            {
                                if (value < GetTerrainId(TREES))
                                {
                                    if (value < GetTerrainId(ROCKS))
                                    {
                                        return SNOW_PEAK;
                                    }

                                    return ROCKS;
                                }

                                return TREES;
                            }

                            return DIRTLAND;
                        }

                        return GRASSLAND;
                    }

                    return ROADS;
                }

                return CITY;
            }

            return SAND;
        }

        return RIVER;
    }

    return LAKE;
}

std::string PaletteWindow::GetToolName(Tool type)
{
    switch (type)
    {
    case ERASE: return "Eraser";
    case SQUARE_BRUSH: return "Sqr Brush";
    case CIRCLE_BRUSH: return "Cir Brush";
    default: "Unknown Tool!";
    }
}

std::string PaletteWindow::GetTerrainName(TerrainType type)
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

int PaletteWindow::GetToolRadius() const
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
    isAlive = true;
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
    while (!isAlive)
    {
        sf::sleep(sf::milliseconds(10));
    }
}

void PaletteWindow::Stop()
{
    isAlive = false;
    executionThread->join();
    delete executionThread;
}
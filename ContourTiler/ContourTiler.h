#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <string>
#include "ColorMapper.h"
#include "Definitions.h"
#include "LineStripLoader.h"
#include "Rasterizer.h"

// Handles startup and the base graphics rendering loop.
class ContourTiler
{
    int size;

    bool rerender;
    sf::Vector2i mouseStart;
    sf::Vector2i mousePos;
    sf::RectangleShape zoomShape;

    decimal leftOffset;
    decimal topOffset;
    decimal effectiveSize;

    double minElevation, maxElevation;
    double* rasterizationBuffer;
    Rasterizer rasterizer;
    LineStripLoader lineStripLoader;

    bool colorize, rescale, lines;
    ColorMapper colorMapper;
    
    sf::Texture overallTexture;
    sf::Sprite overallSprite;
    void CreateOverallTexture();
    void FillOverallTexture();
    void UpdateTextureFromBuffer();
    
    // Handles GUI-based events, such as closing the application, resizing the window, etc.
    void HandleEvents(sf::RenderWindow& window, bool& alive);

    // Renders the scene.
    void Render(sf::RenderWindow& window);

public:
    // Initializes the ContourTiler
    ContourTiler();
    virtual ~ContourTiler();

    // Runs the game loop.
    void Run();
};
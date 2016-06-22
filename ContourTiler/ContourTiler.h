#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <string>
#include "ColorMapper.h"
#include "Definitions.h"
#include "Rasterizer.h"

// Handles startup and the base graphics rendering loop.
class ContourTiler
{
    // All the contours
    int xMax;
    int yMax;
    int zMax;
    std::vector<LineStrip> lineStrips;

    int width;
    int height;

    bool rerender;
    sf::Vector2i mouseStart;
    sf::Vector2i mousePos;
    sf::RectangleShape zoomShape;
    sf::Rect<double> boundingBox;

    double minElevation, maxElevation;
    double* rasterizationBuffer;
    Rasterizer rasterizer;

    bool colorize, rescale;
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
#pragma once

#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <string>
#include "Definitions.h"

// Handles startup and the base graphics rendering loop.
class ContourTiler
{
    sf::Texture overallTexture;
    sf::Sprite overallSprite;

    // All the contours
    std::vector<LineStrip> lineStrips;

    void SimulateRasterization(int width, int height);

    // Handles GUI-based events, such as closing the application, resizing the window, etc.
    void HandleEvents(sf::RenderWindow& window, bool& alive);

    // Renders the scene.
    void Render(sf::RenderWindow& window);

public:
    // Initializes agow and any construction-time setup (such as threads).
    ContourTiler();

    // Runs the game loop.
    void Run();
};
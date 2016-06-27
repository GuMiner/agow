#include <array>
#include <algorithm>
#include <iostream>
#include <future>
#include <limits>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "ContourTiler.h"

#ifndef _DEBUG
    #pragma comment(lib, "lib/sfml-system")
    #pragma comment(lib, "lib/sfml-window")
    #pragma comment(lib, "lib/sfml-graphics")
#else
    #pragma comment(lib, "lib/sfml-system-d")
    #pragma comment(lib, "lib/sfml-window-d")
    #pragma comment(lib, "lib/sfml-graphics-d")
#endif

ContourTiler::ContourTiler() // Size must be divisible by 7.
    : lineStripLoader(), size(1050), rasterizer(&lineStripLoader, size), minElevation(0), maxElevation(1), rasterizationBuffer(new double[size * size]),
    leftOffset(0.0703289), topOffset(0.27929), effectiveSize(0.0188231f), mouseStart(-1, -1), mousePos(-1, -1),
    rerender(false), colorize(false), rescale(false), lines(true)
{ }

ContourTiler::~ContourTiler()
{
    delete[] rasterizationBuffer;
}

void ContourTiler::HandleEvents(sf::RenderWindow& window, bool& alive)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            alive = false;
        }
        else if (event.type == sf::Event::KeyReleased)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                alive = false;
            }
            else if (event.key.code == sf::Keyboard::R)
            {
                // Reset.
                topOffset = 0.0f;
                leftOffset = 0.0f;
                effectiveSize = 1.0f;
                std::cout << "Reset! " << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
            }
            else if (event.key.code == sf::Keyboard::L)
            {
                // Draw lines
                lines = !lines;
                std::cout << "Lines: " << lines << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
            }
            else if (event.key.code == sf::Keyboard::C)
            {
                // Colorize (true/false)
                colorize = !colorize;
                std::cout << "Toggled colorize: " << rescale << std::endl;
                UpdateTextureFromBuffer();
            }
            else if (event.key.code == sf::Keyboard::S)
            {
                rescale = !rescale;
                std::cout << "Toggled rescale: " << rescale << std::endl;
                UpdateTextureFromBuffer();
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // Zoom-in.
                mouseStart = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                mousePos = mouseStart;
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                // Zoom-out.
                topOffset -= effectiveSize * 0.50f;
                leftOffset -= effectiveSize * 0.50f;
                effectiveSize *= 3.0f;

                // Clamp.
                if (topOffset < 0)
                {
                    topOffset = 0;
                }
                if (leftOffset < 0)
                {
                    topOffset = 0;
                }
                if (topOffset + effectiveSize > 1)
                {
                    effectiveSize = 1 - topOffset;
                }
                if (leftOffset + effectiveSize > 1)
                {
                    effectiveSize = 1 - leftOffset;
                }

                std::cout << "Using a new bounding box of [" << leftOffset << ", " << topOffset << ", " << effectiveSize << ", " << effectiveSize << "]" << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
            }
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                int xNew = event.mouseButton.x;
                int yNew = event.mouseButton.y;
                if (xNew > mouseStart.x && yNew > mouseStart.y)
                {
                    // We have a valid zoom-in. Determine the new bounding box. However, we want a proper scaling factor.
                    double scalingFactor = std::min(((double)(xNew - mouseStart.x) / (double)size), ((double)(yNew - mouseStart.y / (double)size)));

                    leftOffset += ((double)mouseStart.x / (double)size) * effectiveSize;
                    topOffset += ((double)mouseStart.y / (double)size) * effectiveSize;
                    effectiveSize = scalingFactor * effectiveSize;
                    std::cout << "Using a new bounding box of [" << leftOffset << ", " << topOffset << ", " << effectiveSize << ", " << effectiveSize << std::endl;
                    sf::sleep(sf::milliseconds(500));
                    rerender = true;
                }

                mouseStart = sf::Vector2i(-1, -1);
            }
        }
    }
}

void ContourTiler::CreateOverallTexture()
{
    overallTexture.create(size, size);
    overallTexture.setRepeated(false);
    overallTexture.setSmooth(false);

    overallSprite.setTexture(overallTexture);

    // Also create the zoom shape.
    zoomShape.setOutlineColor(sf::Color::Green);
    zoomShape.setOutlineThickness(1);
    zoomShape.setFillColor(sf::Color::Transparent);
}

void ContourTiler::FillOverallTexture()
{
    // Rasterize
    rasterizer.Rasterize(leftOffset, topOffset, effectiveSize, &rasterizationBuffer, minElevation, maxElevation);

    if (lines)
    {
        rasterizer.LineRaster(leftOffset, topOffset, effectiveSize, &rasterizationBuffer);
    }

    UpdateTextureFromBuffer();
}

void ContourTiler::UpdateTextureFromBuffer()
{
    // Copy over to the image with an appropriate color mapping.
    sf::Uint8* pixels = new sf::Uint8[size * size * 4]; // * 4 because pixels have 4 components (RGBA)
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            int pixelIdx = (i + j * size) * 4;
            double elevation = rasterizationBuffer[i + j * size];
            if (elevation < minElevation || elevation > maxElevation)
            {
                pixels[pixelIdx] = 255;
                pixels[pixelIdx + 1] = 255;
                pixels[pixelIdx + 2] = 0;
            }
            else
            {
                double elevationPercent = rescale ? (elevation - minElevation) / (maxElevation - minElevation) : elevation;
                if (colorize)
                {
                    colorMapper.MapColor(elevationPercent, &pixels[pixelIdx], &pixels[pixelIdx + 1], &pixels[pixelIdx + 2]);
                }
                else
                {
                    int z = (int)(elevationPercent * 255);

                    pixels[pixelIdx] = z;
                    pixels[pixelIdx + 1] = z;
                    pixels[pixelIdx + 2] = z;
                }
            }

            pixels[pixelIdx + 3] = 255;
        }
    }

    overallTexture.update(pixels);
    delete[] pixels;
}

void ContourTiler::Render(sf::RenderWindow& window)
{
    window.clear(sf::Color::Blue);

    if (rerender)
    {
        FillOverallTexture();
        rerender = false;
    }

    window.draw(overallSprite);

    if (mouseStart.x != -1)
    {
        zoomShape.setPosition(sf::Vector2f((float)mouseStart.x, (float)mouseStart.y));
        float minDiff = std::min((float)(mousePos.x - mouseStart.x), (float)(mousePos.y - mouseStart.y));
        zoomShape.setSize(sf::Vector2f(minDiff, minDiff));
        window.draw(zoomShape);
    }
}

void ContourTiler::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style =  sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(size, size), "Contour Tiler", style, contextSettings);
    window.setFramerateLimit(60);

    // Load our data file.
    if (!lineStripLoader.Initialize(Constant::ContourFile))
    {
        std::cout << "Could not read the line strips file!" << std::endl;
        return;
    }

    rasterizer.Setup();

    CreateOverallTexture();
    rerender = true;

    // Start the main loop
    bool alive = true;
    while (alive)
    {
        HandleEvents(window, alive);
        Render(window);

        // Display what we rendered.
        window.display();
    }
}

// Performs the graphical interpolation and tiling of contours.
int main(int argc, char* argv[])
{
    std::cout << "ContourTiler Start!" << std::endl;

    std::unique_ptr<ContourTiler> contourTiler(new ContourTiler());
    contourTiler->Run();

    std::cout << "ContourTiler End!" << std::endl;
    sf::sleep(sf::milliseconds(1000));

    return 0;
}
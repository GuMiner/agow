#include <array>
#include <algorithm>
#include <cmath>
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

ContourTiler::ContourTiler() // Size must be divisible by 5.
    : lineStripLoader(), quadExclusions(), size(1000), rasterizer(&lineStripLoader, &quadExclusions, size), minElevation(0), maxElevation(1), rasterizationBuffer(new double[size * size]), linesBuffer(new double[size * size]),
      leftOffset((decimal)0.169734), topOffset((decimal)0.291619), effectiveSize((decimal)0.0181266), mouseStart(-1, -1), mousePos(-1, -1), isRendering(false),
      rerender(false), colorize(false), rescale(false), lines(true), hideExclusionShape(false)
{ }

ContourTiler::~ContourTiler()
{
    delete[] rasterizationBuffer;
    delete[] linesBuffer;
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
            }
            else if (event.key.code == sf::Keyboard::C)
            {
                // Colorize (true/false)
                colorize = !colorize;
                std::cout << "Toggled colorize: " << rescale << std::endl;
            }
            else if (event.key.code == sf::Keyboard::S)
            {
                rescale = !rescale;
                std::cout << "Toggled rescale: " << rescale << std::endl;
            }
            else if (event.key.code == sf::Keyboard::U)
            {
                std::cout << "Forced an update." << std::endl;
                rerender = true;
            }
            else if (event.key.code == sf::Keyboard::A)
            {
                decimal x = leftOffset + (mousePos.x / (decimal)size) * effectiveSize;
                decimal y = topOffset + (mousePos.y / (decimal)size) * effectiveSize;

                sf::Vector2i point(std::min((int)(x * size), size - 1), std::min((int)(y * size), size - 1));
                std::cout << "Toggled point [" << point.x << ", " << point.y << "] to " << quadExclusions.ToggleExclusion(point) << std::endl;
            }
            else if (event.key.code == sf::Keyboard::W)
            {
                quadExclusions.WriteExclusions();
            }
            else if (event.key.code == sf::Keyboard::H)
            {
                hideExclusionShape = !hideExclusionShape;
                std::cout << "Toggled hiding the exclusion shape." << std::endl;
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
                    leftOffset = 0;
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
            // Update for the zoom rectangle and exclusion shape.
            mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);

            decimal x = leftOffset + (mousePos.x / (decimal)size) * effectiveSize;
            decimal y = topOffset + (mousePos.y / (decimal)size) * effectiveSize;

            sf::Vector2i point(std::min((int)(x * size), size - 1), std::min((int)(y * size), size - 1));
            sf::Vector2i nextPoint(point.x + 1, point.y + 1);

            decimal startX = ((((decimal)point.x / (decimal)size) - leftOffset) / effectiveSize) * size;
            decimal startY = ((((decimal)point.y / (decimal)size) - topOffset) / effectiveSize) * size;
            decimal endX = ((((decimal)nextPoint.x / (decimal)size) - leftOffset) / effectiveSize) * size;
            decimal endY = ((((decimal)nextPoint.y / (decimal)size) - topOffset) / effectiveSize) * size;

            sf::Vector2f size(sf::Vector2f(std::abs(startX - endX), std::abs(startY - endY)));
            exclusionShape.setPosition(sf::Vector2f(startX, startY));
            exclusionShape.setSize(size);
            // std::cout << "Moved exclusion shape to [" << startX << ", " << startY << "], scaled to [" << size.x << ", " << size.y << "]." << std::endl;
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
                    decimal scalingFactor = std::min(((decimal)(xNew - mouseStart.x) / (decimal)size), ((decimal)(yNew - mouseStart.y / (decimal)size)));

                    leftOffset += ((decimal)mouseStart.x / (decimal)size) * effectiveSize;
                    topOffset += ((decimal)mouseStart.y / (decimal)size) * effectiveSize;
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

void ContourTiler::SetupGraphicsElements()
{
    overallTexture.create(size, size);
    overallTexture.setRepeated(false);
    overallTexture.setSmooth(false);

    overallSprite.setTexture(overallTexture);

    // Also create the zoom shape.
    zoomShape.setOutlineColor(sf::Color::Green);
    zoomShape.setOutlineThickness(1);
    zoomShape.setFillColor(sf::Color::Transparent);

    // And the exclusion shape.
    exclusionShape.setFillColor(sf::Color(0, 0, 255, 140));
}

void ContourTiler::FillOverallTexture()
{
    // Rasterize
    rasterizer.Rasterize(leftOffset, topOffset, effectiveSize, &rasterizationBuffer, minElevation, maxElevation);
    if (lines)
    {
        rasterizer.LineRaster(leftOffset, topOffset, effectiveSize, &linesBuffer);
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
            double elevation = rasterizationBuffer[i + j * size];
            double elevationPercent = rescale ? (elevation - minElevation) / (maxElevation - minElevation) : elevation;
            int pixelIdx = (i + j * size) * 4;

            if (elevation > 1)
            {
                // Exclusion zones.
                pixels[pixelIdx] = 200;
                pixels[pixelIdx + 1] = 0;
                pixels[pixelIdx + 2] = 230;
            }
            else
            {
                // Rasterization buffer.
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

            // Lines buffer modification, only if applicable.
            if (lines && linesBuffer[i + j * size] > 0.5)
            {
                pixels[pixelIdx] = std::min(255, pixels[pixelIdx] + 50);
                pixels[pixelIdx + 1] = std::min(255, pixels[pixelIdx] + 50);
            }

            pixels[pixelIdx + 3] = 255;
        }
    }

    overallTexture.update(pixels);
    delete[] pixels;
}

void ContourTiler::Render(sf::RenderWindow& window, sf::Time elapsedTime)
{
    // Rerender as needed on a separate thread.
    if (rerender && !isRendering)
    {
        isRendering = true;
        rasterStartTime = elapsedTime;
        renderingThread = std::async(std::launch::async, &ContourTiler::FillOverallTexture, this);
    }
    else if (isRendering)
    {
        std::future_status status = renderingThread.wait_until(std::chrono::system_clock::now());
        if (status == std::future_status::ready)
        {
            rerender = false;
            isRendering = false;
            std::cout << "Raster time: " << (elapsedTime - rasterStartTime).asSeconds() << " s." << std::endl;
        }
    }

    // Update the texture at a reasonable but not too fast pace.
    if (elapsedTime - lastUpdateTime > sf::milliseconds(333))
    {
        lastUpdateTime = elapsedTime;
        UpdateTextureFromBuffer();
    }

    // Draw the raster result and our zoom box if applicable.
    window.clear(sf::Color::Blue);
    window.draw(overallSprite);

    if (mouseStart.x != -1)
    {
        zoomShape.setPosition(sf::Vector2f((float)mouseStart.x, (float)mouseStart.y));
        float minDiff = std::min((float)(mousePos.x - mouseStart.x), (float)(mousePos.y - mouseStart.y));
        zoomShape.setSize(sf::Vector2f(minDiff, minDiff));
        window.draw(zoomShape);
    }

    if (!hideExclusionShape)
    {
        window.draw(exclusionShape);
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

    // Load the exclusion file.
    quadExclusions.ReadExclusions();

    rasterizer.Setup();

    SetupGraphicsElements();
    rerender = true;

    // Start the main loop
    bool alive = true;
    sf::Clock timer;
    lastUpdateTime = timer.getElapsedTime();
    while (alive)
    {
        HandleEvents(window, alive);
        Render(window, timer.getElapsedTime());

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
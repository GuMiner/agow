#include <array>
#include <algorithm>
#include <cmath>
#include <direct.h>
#include <iostream>
#include <future>
#include <limits>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <stb/stb_image_write.h>
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

ContourTiler::ContourTiler()
    : lineStripLoader(), quadExclusions(), size(1000), regionSize(70), rasterizer(&lineStripLoader, &quadExclusions, size), minElevation(0), maxElevation(1), rasterizationBuffer(new double[size * size]), linesBuffer(new double[size * size]), coverBuffer(new bool[size * size]),
      leftOffset((decimal)0.64), topOffset((decimal)0.14), effectiveSize((decimal)0.08), mouseStart(-1, -1), mousePos(-1, -1), isRendering(false), isZoomMode(true),
      rerender(false), viewOptions(), hideExclusionShape(false), isBulkProcessing(false), regionX(21), regionY(2)
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
                viewOptions.lines = !viewOptions.lines;
                std::cout << "Lines: " << viewOptions.lines << std::endl;
            }
            else if (event.key.code == sf::Keyboard::C)
            {
                // Colorize (true/false)
                viewOptions.colorize = !viewOptions.colorize;
                std::cout << "Toggled colorize: " << viewOptions.colorize << std::endl;
            }
            else if (event.key.code == sf::Keyboard::S)
            {
                viewOptions.rescale = !viewOptions.rescale;
                std::cout << "Toggled rescale: " << viewOptions.rescale << std::endl;
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
                bool exclusionStatus = quadExclusions.ToggleExclusion(point);
                std::cout << "Toggled point [" << point.x << ", " << point.y << "] to " << exclusionStatus << std::endl;

                // Update our temporary display accordingly.
                int xStart = (int)exclusionShape.getPosition().x;
                int yStart = (int)exclusionShape.getPosition().y;
                int xEnd = xStart + (int)exclusionShape.getSize().x;
                int yEnd = yStart + (int)exclusionShape.getSize().y;
                for (int i = xStart; i < xEnd; i++)
                {
                    for (int j = yStart; j < yEnd; j++)
                    {
                        coverBuffer[i + j * size] = exclusionStatus;
                    }
                }
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
            else if (event.key.code == sf::Keyboard::Z)
            {
                isZoomMode = !isZoomMode;
                std::cout << "Toggled zoom/exclude mode: " << isZoomMode << "." << std::endl;
            }
            else if (event.key.code == sf::Keyboard::P)
            {
                // Bulk processing divides the area into 3-ft resolution areas 70x70 all 1000x1000 pixels.
                std::cout << "Starting bulk processing mode." << std::endl;
                
                // Point of no return -- this continues until done.
                ZoomToRegion(regionX, regionY);
                isBulkProcessing = true;
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
                    if (isZoomMode)
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
                    else
                    {
                        // We have a valid exclusion. Exclude all the points within the bounding box.
                        decimal xStart = leftOffset + (mouseStart.x / (decimal)size) * effectiveSize;
                        decimal yStart = topOffset + (mouseStart.y / (decimal)size) * effectiveSize;
                        decimal xEnd = leftOffset + (mousePos.x / (decimal)size) * effectiveSize;
                        decimal yEnd = topOffset + (mousePos.y / (decimal)size) * effectiveSize;

                        sf::Vector2i pointStart(std::min((int)(xStart * size), size - 1), std::min((int)(yStart * size), size - 1));
                        sf::Vector2i pointEnd(std::min((int)(xEnd * size), size - 1), std::min((int)(yEnd * size), size - 1));
                        
                        for (int x = std::min(pointStart.x, pointEnd.x); x <= std::max(pointStart.x, pointEnd.x); x++)
                        {
                            for (int y = std::min(pointStart.y, pointEnd.y); y <= std::max(pointStart.y, pointEnd.y); y++)
                            {
                                sf::Vector2i point(x, y);
                                if (!quadExclusions.ToggleExclusion(point))
                                {
                                    quadExclusions.ToggleExclusion(point);
                                }
                            }

                            std::cout << "Excluded point range [" << x << ", [" << pointStart.y << ", " << pointEnd.y << "]]." << std::endl;
                        }
                        
                        // Now update the visible display, but in a less strict manner for basic setup.
                        for (int i = mouseStart.x; i < mousePos.x; i++)
                        {
                            for (int j = mouseStart.y; j < mousePos.y; j++)
                            {
                                coverBuffer[i + j * size] = true;
                            }
                        }
                    }
                }

                mouseStart = sf::Vector2i(-1, -1);
            }
        }
    }
}

void ContourTiler::ZoomToRegion(int x, int y)
{
    decimal viewSize = (decimal)1.0 / (decimal)regionSize;
    leftOffset = (decimal)x * viewSize;
    topOffset = (decimal)y * viewSize;
    effectiveSize = viewSize;
    rerender = true;
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
    if (viewOptions.lines)
    {
        rasterizer.LineRaster(leftOffset, topOffset, effectiveSize, &linesBuffer);
    }

    UpdateTextureFromBuffer();
}

void ContourTiler::ClearCoverPane()
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            coverBuffer[i + j*size] = false;
        }
    }
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
            double elevationPercent = viewOptions.rescale ? (elevation - minElevation) / (maxElevation - minElevation) : elevation;
            int pixelIdx = (i + j * size) * 4;

            if (elevation > 1 || coverBuffer[i + j * size])
            {
                // Exclusion zones.
                pixels[pixelIdx] = 200;
                pixels[pixelIdx + 1] = 0;
                pixels[pixelIdx + 2] = 230;
            }
            else
            {
                // Rasterization buffer.
                if (viewOptions.colorize)
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
            if (viewOptions.lines && linesBuffer[i + j * size] > 0.5)
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
        ClearCoverPane();
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

            if (isBulkProcessing)
            {
                // Create a new folder if we haven't already
                if (regionX == regionSize - 1 || (regionX == 0 && regionY == 0))
                {
                    std::stringstream folder;
                    folder << ".\\rasters\\" << regionY;
                    _mkdir(folder.str().c_str());
                }

                // Save out our current data
                std::stringstream file;
                file << "rasters/" << regionY << "/" << regionX << ".png";

                unsigned char* data = new unsigned char[size * size * 4];
                for (int i = 0; i < size; i++)
                {
                    for (int j = 0; j < size; j++)
                    {
                        // RGBA order
                        int scaledVersion = std::min((int)(rasterizationBuffer[i + j * size] * (65536)), 65535);
                        // RED == upper 8 bytes.
                        // GREEN == lower 8 bytes.

                        data[(i + j * size) * 4] = (unsigned char)(scaledVersion & 0x00FF);
                        data[(i + j * size) * 4 + 1] = (unsigned char)((scaledVersion & 0xFF00) >> 8);
                        data[(i + j * size) * 4 + 2] = 255;
                        data[(i + j * size) * 4 + 3] = 255;
                    }
                }

                const int RGBA = 4;
                int result = stbi_write_png(file.str().c_str(), size, size, RGBA, &data[0], size * 4 * sizeof(unsigned char));
                if (result != 0)
                {
                    std::cout << "Failed to write a file: " << result << " for raster " << regionX << ", " << regionY << std::endl;
                }
                
                std::cout << "Wrote the file " << regionX << ", " << regionY << std::endl;
                delete[] data;

                // Move to the next region.
                regionX++;
                if (regionX == regionSize - 1)
                {
                    regionX = 0;
                    regionY++;
                }

                // Continue;
                if (regionY != regionSize)
                {
                    ZoomToRegion(regionX, regionY);
                }
                else
                {
                    isBulkProcessing = false;
                }
            }
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
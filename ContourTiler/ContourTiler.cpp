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
#include "LineStripLoader.h"

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
    : xMax(100000), yMax(100000), zMax(3000), rasterizer(xMax, yMax, 800),
      width(900), height(675), boundingBox(sf::Rect<double>(0.00584145, 0.0146719, 0.635762, 0.635762)),
      rerender(false), mouseStart(-1, -1), mousePos(-1, -1), colorize(false), rescale(false)
{
}

ContourTiler::~ContourTiler()
{
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
                boundingBox = sf::Rect<double>(0, 0, 1, 1);
                std::cout << "Reset! " << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
            }
            else if (event.key.code == sf::Keyboard::C)
            {
                // Colorize (true/false)
                colorize = !colorize;
                std::cout << "Toggled colorize: " << rescale << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
            }
            else if (event.key.code == sf::Keyboard::S)
            {
                rescale = !rescale;
                std::cout << "Toggled rescale: " << rescale << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
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
                boundingBox.left = boundingBox.left / 2;
                boundingBox.top = boundingBox.top / 2;
                boundingBox.width = boundingBox.width * 2;
                boundingBox.height = boundingBox.height * 2;

                // Clamp.
                if (boundingBox.left < 0)
                {
                    boundingBox.left = 0;
                }
                if (boundingBox.top < 0)
                {
                    boundingBox.top = 0;
                }
                if (boundingBox.width + boundingBox.left > 1)
                {
                    boundingBox.width = 1 - boundingBox.left;
                }
                if (boundingBox.height + boundingBox.top > 1)
                {
                    boundingBox.height = 1 - boundingBox.top;
                }

                std::cout << "Using a new bounding box of [" << boundingBox.left << ", " << boundingBox.top << ", " << boundingBox.width << ", " << boundingBox.height << std::endl;
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
                    double scalingFactor = ((double)(xNew - mouseStart.x) / (double)width);

                    boundingBox.left = boundingBox.left + ((double)mouseStart.x / (double)width) * boundingBox.width;
                    boundingBox.top = boundingBox.top + (1.0 - ((double)yNew / (double)height)) * boundingBox.height;
                    boundingBox.width = scalingFactor * boundingBox.width;
                    boundingBox.height = scalingFactor * boundingBox.height;
                    std::cout << "Using a new bounding box of [" << boundingBox.left << ", " << boundingBox.top << ", " << boundingBox.width << ", " << boundingBox.height << std::endl;
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
    overallTexture.create(width, height);
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
    double minElevation, maxElevation;
    double* points = new double[width * height];
    rasterizer.Rasterize(boundingBox, width, height, &points, minElevation, maxElevation);

    // Copy over to the image with an appropriate color mapping.
    sf::Uint8* pixels = new sf::Uint8[width * height * 4]; // * 4 because pixels have 4 components (RGBA)
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double elevationPercent = rescale ?
                ((points[i + j * width] - minElevation) / (maxElevation - minElevation)) :
                (points[i + j * width] / zMax);

            int pixelIdx = (i + j * width) * 4;
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
        zoomShape.setSize(sf::Vector2f((float)(mousePos.x - mouseStart.x), (float)(mousePos.y - mouseStart.y)));
        window.draw(zoomShape);
    }
}

void ContourTiler::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style =  sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(width, height), "Contour Tiler", style, contextSettings);
    window.setFramerateLimit(60);

    // Load our data file.
    if (!LineStripLoader::LoadLineStrips(Constant::ContourFile, &lineStrips))
    {
        std::cout << "Could not read the line strips file!" << std::endl;
        return;
    }

    rasterizer.Setup(&lineStrips);

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
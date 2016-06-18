#include <iostream>
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
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            // if (event.mouseButton.button == sf::Mouse::Left
        }
    }
}

void ContourTiler::Render(sf::RenderWindow& window)
{
    window.clear(sf::Color::Blue);
    window.draw(overallSprite);
}

void ContourTiler::SimulateRasterization(int width, int height)
{
    sf::Uint8* pixels = new sf::Uint8[width * height * 4]; // * 4 because pixels have 4 components (RGBA)
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int pixelIdx = (i + j * width) * 4;

            // Find all points that 
            pixels[pixelIdx] = 0;
            pixels[pixelIdx + 1] = 0;
            pixels[pixelIdx + 2] = 0;

            pixels[pixelIdx + 3] = 255;
        }
    }

    // 100,000 | 100,000 | 3,000
    int xMax = 100000;
    int yMax = 100000;
    int zMax = 3000;
    for (unsigned int i = 0; i < lineStrips.size(); i++)
    {
        for (unsigned int j = 0; j < lineStrips[i].points.size(); j++)
        {
            int x = (int)((lineStrips[i].points[j].x / xMax) * (width - 1));
            int y = (height - 1) - (int)((lineStrips[i].points[j].y / yMax) * (height - 1));
            int z = (int)((lineStrips[i].elevation / zMax) * 255);

            if (x < 0 || y < 0 || x >= width || y >= height || z >= 256)
            {
                std::cout << "Hit bounds: " << x << " " << y << " " << z << std::endl;
            }

            int pixelIdx = (x + y * width) * 4;

            if (pixels[pixelIdx] < z)
            {
                pixels[pixelIdx] = z;
                pixels[pixelIdx + 1] = pixels[pixelIdx + 1] != 0 ? pixels[pixelIdx + 1] : z;
                pixels[pixelIdx + 2] = z;
            }
        }
    }

    overallTexture.update(pixels);
    delete[] pixels;
}

void ContourTiler::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style =  sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Contour Tiler", style, contextSettings);
    window.setFramerateLimit(60);

    // Perform pre-init.
    std::cout << "Loading line strips..." << std::endl;
    if (!LineStripLoader::LoadLineStrips("contours.bin", lineStrips))
    {
        std::cout << "Line strip loading failed!" << std::endl;
    }

    std::cout << "Line strips loaded!" << std::endl;

    int tWidth = 800;
    int tHeight = 600;

    overallTexture.create(tWidth, tHeight);
    overallTexture.setRepeated(false);
    overallTexture.setSmooth(false);

    overallSprite.setTexture(overallTexture);

    SimulateRasterization(tWidth, tHeight);

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

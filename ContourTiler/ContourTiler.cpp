#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "ContourTiler.h"

#ifndef _DEBUG
    #pragma comment(lib, "../lib/sfml-system")
    #pragma comment(lib, "../lib/sfml-window")
    #pragma comment(lib, "../lib/sfml-graphics")
#else
    #pragma comment(lib, "../lib/sfml-system-d")
    #pragma comment(lib, "../lib/sfml-window-d")
    #pragma comment(lib, "../lib/sfml-graphics-d")
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
    window.clear(sf::Color::Green);

    // TODO
}

void ContourTiler::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style =  sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Advanced Graphics-Open World", style, contextSettings);
    window.setFramerateLimit(60);

    sf::Time clockStartTime;
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

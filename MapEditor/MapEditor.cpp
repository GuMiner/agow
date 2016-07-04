#include <iostream>
#include <thread>
#include <stb/stb_image.h>
#include "MapEditor.h"

#ifndef _DEBUG
#pragma comment(lib, "../lib/sfml-system")
#pragma comment(lib, "../lib/sfml-window")
#pragma comment(lib, "../lib/sfml-graphics")
#else
#pragma comment(lib, "../lib/sfml-system-d")
#pragma comment(lib, "../lib/sfml-window-d")
#pragma comment(lib, "../lib/sfml-graphics-d")
#endif

MapEditor::MapEditor()
    : size(900), tileCount(70), tileSize(1000), summaryView(700, tileCount, tileSize / (700 / tileCount)), paletteWindow(200), rawTileData(nullptr)
{
}

void MapEditor::LoadGraphics()
{
    // The summary view performs a bunch of graphics loading.
    summaryView.Start();

    currentTileTexture.create(tileSize, tileSize);
    currentTileTexture.setRepeated(false);
    currentTileTexture.setSmooth(false);

    currentTile.setTextureRect(sf::IntRect(0, size, size, -size));
    currentTile.setTexture(currentTileTexture);
    UpdateCurrentTileTexture();

    paletteWindow.Start();
}

void MapEditor::HandleEvents(sf::RenderWindow& window, bool& alive)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            alive = false;
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            bool tileChanged = false;
            switch (event.key.code)
            {
            case sf::Keyboard::Left:  summaryView.MoveSelectedTile(SummaryView::Direction::LEFT);  tileChanged = true; break;
            case sf::Keyboard::Right: summaryView.MoveSelectedTile(SummaryView::Direction::RIGHT); tileChanged = true; break;
            case sf::Keyboard::Up:    summaryView.MoveSelectedTile(SummaryView::Direction::UP);    tileChanged = true; break;
            case sf::Keyboard::Down:  summaryView.MoveSelectedTile(SummaryView::Direction::DOWN);  tileChanged = true; break;
            default: break;
            }

            if (tileChanged)
            {
                UpdateCurrentTileTexture();
            }
        }
    }
}

void MapEditor::UpdateCurrentTileTexture()
{
    summaryView.LoadSelectedTile(&rawTileData);
    currentTileTexture.update(rawTileData);
}

void MapEditor::Render(sf::RenderWindow& window)
{
    window.draw(currentTile);
}

void MapEditor::Run()
{
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(size, size), "Map Editor", style, contextSettings);
    window.setFramerateLimit(60);

    // Setup our graphics elements.
    LoadGraphics();

    // Start the main loop
    bool alive = true;
    while (alive)
    {
        HandleEvents(window, alive);
        Render(window);

        // Display what we rendered.
        window.display();
    }

    // TODO save out the currently-open tile and delete the data.
    summaryView.Stop();

    paletteWindow.Stop();
}

int main()
{
    std::cout << "MapEditor Start!" << std::endl;

    std::unique_ptr<MapEditor> contourTiler(new MapEditor());
    contourTiler->Run();

    std::cout << "MapEditor End!" << std::endl;
    sf::sleep(sf::milliseconds(1000));

    return 0;
}


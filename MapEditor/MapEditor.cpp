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
    : size(900), tileCount(70), tileSize(1000), summaryView(700, tileCount, tileSize / (700 / tileCount)), paletteWindow(200), rawTileData(nullptr), 
      mouseDown(false), displaySettings()
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
            case sf::Keyboard::S:
                // TODO -- save out the current tile with any modifications.
                break;
            case sf::Keyboard::R: displaySettings.rescale = !displaySettings.rescale; std::cout << "Rescale: " << displaySettings.rescale << std::endl; tileChanged = true; break;
            case sf::Keyboard::C: displaySettings.showContours = !displaySettings.showContours; std::cout << "Contours: " << displaySettings.showContours << std::endl; tileChanged = true; break;
            case sf::Keyboard::O: displaySettings.showOverlay = !displaySettings.showOverlay; std::cout << "Overlay: " << displaySettings.showOverlay << std::endl; tileChanged = true; break;
            default: break;
            }

            if (tileChanged)
            {
                UpdateCurrentTileTexture();
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            mouseDown = true;
            // TODO Do some drawing.

            UpdateCurrentTile();
        }
        else if (event.type == sf::Event::MouseMoved && mouseDown)
        {
            // TODO Do some drawing.
            UpdateCurrentTile();
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            mouseDown = false;
        }
    }
}

void MapEditor::UpdateCurrentTile()
{
    sf::Uint8* convertedRawData = new sf::Uint8[tileSize * tileSize * 4];
    
    // Determine the limits of the current tile.
    unsigned short minValue = std::numeric_limits<unsigned short>::max();
    unsigned short maxValue = 0;
    for (int x = 0; x < tileSize; x++)
    {
        for (int y = 0; y < tileSize; y++)
        {
            int pos = (x + y * tileSize) * 4;
            unsigned short height = (unsigned short)rawTileData[pos] + (((unsigned short)rawTileData[pos + 1]) << 8);
            if (height > maxValue)
            {
                maxValue = height;
            }
            if (height < minValue)
            {
                minValue = height;
            }
        }
    }

    // Merge in the raw data with our settings.
    for (int x = 0; x < tileSize; x++)
    {
        for (int y = 0; y < tileSize; y++)
        {
            // Rescale if requested.
            int pos = (x + y * tileSize) * 4;
            unsigned char rawValue;
            unsigned short height = (unsigned short)rawTileData[pos] + (((unsigned short)rawTileData[pos + 1]) << 8);
            if (displaySettings.rescale)
            {
                rawValue = (unsigned char)((float)(height - minValue) / (float)(maxValue - minValue) * 255);
            }
            else
            {
                rawValue = (unsigned char)(height / 256);
            }

            // Add contour lines if requested.
            bool contourLineHere = (displaySettings.showContours && (rawTileData[pos] == 1 || rawTileData[pos] == 0));
            
            convertedRawData[pos] = contourLineHere ? 255 : rawValue;
            convertedRawData[pos + 1] = contourLineHere ? 255 : rawValue;
            convertedRawData[pos + 2] = rawValue;
            convertedRawData[pos + 3] = 255;

            // Show the overlay if requested.
            if (displaySettings.showOverlay)
            {
                // empirically determined
                float overlayScale = 0.20f;
                sf::Color overlayColor = PaletteWindow::GetTerrainColor(PaletteWindow::GetNearestTerrainType(rawTileData[pos + 2]));
                convertedRawData[pos] = (unsigned short)((float)overlayColor.r * overlayScale) + (unsigned short)convertedRawData[pos] > 255 ? 255 : (unsigned char)((float)overlayColor.r * overlayScale) + convertedRawData[pos];
                convertedRawData[pos + 1] = (unsigned short)((float)overlayColor.g * overlayScale) + (unsigned short)convertedRawData[pos + 1] > 255 ? 255 : (unsigned char)((float)overlayColor.g * overlayScale) + convertedRawData[pos + 1];
                convertedRawData[pos + 2] = (unsigned short)((float)overlayColor.b * overlayScale) + (unsigned short)convertedRawData[pos + 2] > 255 ? 255 : (unsigned char)((float)overlayColor.b * overlayScale) + convertedRawData[pos + 2];
            }
        }
    }

    currentTileTexture.update(convertedRawData);
    delete[] convertedRawData;
}

void MapEditor::UpdateCurrentTileTexture()
{
    summaryView.LoadSelectedTile(&rawTileData);
    UpdateCurrentTile();
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


#include <cmath>
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
    : size(900), tileCount(70), tileSize(1000), summaryView(700, tileCount, tileSize / (700 / tileCount)), paletteWindow(200), currentTile(),
      mouseDown(false), displaySettings(), brushes((float)paletteWindow.GetToolRadius()), convertedRawData(new sf::Uint8[tileSize * tileSize * 4]), offset(15), saveOnMove(true)
{
}

void MapEditor::CreateSpriteTexturePair(sf::Sprite& sprite, sf::Texture& texture, sf::Vector2f spritePos, sf::IntRect textureRect)
{
    texture.create(tileSize, tileSize);
    texture.setRepeated(false);
    texture.setSmooth(false);

    sprite.setTextureRect(textureRect);
    sprite.setPosition(spritePos);
    sprite.setScale(sf::Vector2f((float)(size - 2 * offset) / (float)tileSize, (float)(size - 2 * offset) / (float)tileSize));
    sprite.setTexture(texture);
}

void MapEditor::LoadGraphics()
{
    // The other two views perform a bunch of graphical loading.
    summaryView.Start();
    paletteWindow.Start();

    CreateSpriteTexturePair(currentTile.centerSprite, currentTile.centerTexture, sf::Vector2f(offset, offset),             sf::IntRect(0, tileSize, tileSize, -tileSize));
    CreateSpriteTexturePair(currentTile.leftSprite, currentTile.leftTexture,     sf::Vector2f(-size + offset*3, offset), sf::IntRect(0, tileSize, tileSize, -tileSize));
    CreateSpriteTexturePair(currentTile.topSprite, currentTile.topTexture,       sf::Vector2f(offset, -size + offset*3), sf::IntRect(0, tileSize, tileSize, -tileSize));
    CreateSpriteTexturePair(currentTile.rightSprite, currentTile.rightTexture,   sf::Vector2f(size - offset, offset),  sf::IntRect(0, tileSize, tileSize, -tileSize));
    CreateSpriteTexturePair(currentTile.bottomSprite, currentTile.bottomTexture, sf::Vector2f(offset, size - offset),  sf::IntRect(0, tileSize, tileSize, -tileSize));

    summaryView.LoadSelectedTile(&currentTile.center, &currentTile.left, &currentTile.right, &currentTile.up, &currentTile.down);
    RedrawCurrentTiles();
}

void MapEditor::HandleEvents(sf::RenderWindow& window, bool& alive)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            // Save data.
            SaveTile();
            alive = false;
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            bool tileChanged = false;
            bool handled = true;
            switch (event.key.code)
            {
            case sf::Keyboard::Left:  if (saveOnMove) { SaveTile(); } summaryView.MoveSelectedTile(SummaryView::Direction::LEFT);  summaryView.LoadSelectedTile(&currentTile.center, &currentTile.left, &currentTile.right, &currentTile.up, &currentTile.down);  tileChanged = true; break;
            case sf::Keyboard::Right: if (saveOnMove) { SaveTile(); } summaryView.MoveSelectedTile(SummaryView::Direction::RIGHT); summaryView.LoadSelectedTile(&currentTile.center, &currentTile.left, &currentTile.right, &currentTile.up, &currentTile.down);  tileChanged = true; break;
            case sf::Keyboard::Up:    if (saveOnMove) { SaveTile(); } summaryView.MoveSelectedTile(SummaryView::Direction::UP);    summaryView.LoadSelectedTile(&currentTile.center, &currentTile.left, &currentTile.right, &currentTile.up, &currentTile.down);  tileChanged = true; break;
            case sf::Keyboard::Down:  if (saveOnMove) { SaveTile(); } summaryView.MoveSelectedTile(SummaryView::Direction::DOWN);  summaryView.LoadSelectedTile(&currentTile.center, &currentTile.left, &currentTile.right, &currentTile.up, &currentTile.down);  tileChanged = true; break;
            case sf::Keyboard::R: displaySettings.rescale = !displaySettings.rescale;               std::cout << "Rescale: " << displaySettings.rescale << std::endl;          tileChanged = true; break;
            case sf::Keyboard::C: displaySettings.showContours = !displaySettings.showContours;     std::cout << "Contours: " << displaySettings.showContours << std::endl;    tileChanged = true; break;
            case sf::Keyboard::O: displaySettings.showOverlay = !displaySettings.showOverlay;       std::cout << "Overlay: " << displaySettings.showOverlay << std::endl;      tileChanged = true; break;
            case sf::Keyboard::S: saveOnMove = !saveOnMove; std::cout << "WARNING: Save on move: " << saveOnMove << std::endl; break;
            default: handled = false; break;
            }

            if (tileChanged)
            {
                RedrawCurrentTiles();
            }

            if (!handled)
            {
                paletteWindow.HandleKeyEvent(event);
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            mouseDown = true;
            Draw(paletteWindow.GetSelectedTool(), (float)paletteWindow.GetToolRadius(), PaletteWindow::GetTerrainId(paletteWindow.GetTerrainType()), event.mouseButton.x, event.mouseButton.y);
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            float radius = (float)paletteWindow.GetToolRadius();
            brushes.circleBrush.setPosition(sf::Vector2f((float)event.mouseMove.x - radius, (float)event.mouseMove.y - radius));
            brushes.squareBrush.setPosition(sf::Vector2f((float)event.mouseMove.x - radius, (float)event.mouseMove.y - radius));

            if (mouseDown)
            {
                Draw(paletteWindow.GetSelectedTool(), (float)paletteWindow.GetToolRadius(), PaletteWindow::GetTerrainId(paletteWindow.GetTerrainType()), event.mouseMove.x, event.mouseMove.y);
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            mouseDown = false;
        }
    }
}

void MapEditor::Draw(PaletteWindow::Tool tool, float radius, unsigned char terrainId, int mouseX, int mouseY)
{
    // Upscale to the image.
    mouseX = (int)((float)mouseX * (float)tileSize / (float)size);
    mouseY = (int)((float)mouseY * (float)tileSize / (float)size);
    mouseY = (tileSize - mouseY);

    int minX = std::max(mouseX - (int)radius, 0);
    int minY = std::max(mouseY - (int)radius, 0);
    int maxX = std::min(mouseX + (int)radius, tileSize - 1);
    int maxY = std::min(mouseY + (int)radius, tileSize - 1);

    if (tool == PaletteWindow::ERASE)
    {
        terrainId = 255;
    }

    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            int tileId = (x + y * tileSize) * 4 + 2;

            bool isInBrushArea = tool == PaletteWindow::SQUARE_BRUSH ||
                tool == PaletteWindow::ERASE ||
                (tool == PaletteWindow::CIRCLE_BRUSH && (pow((float)x - (float)mouseX, 2) + pow((float)y - (float)mouseY, 2) < pow(radius, 2)));

            // We allow overwriting lakes (the default).
            bool isOverwriteAllowed = paletteWindow.IsOverwriteAllowed() || paletteWindow.GetNearestTerrainType(currentTile.center[tileId]) == PaletteWindow::TerrainType::LAKE;

            if (isInBrushArea && isOverwriteAllowed)
            {
                currentTile.center[tileId] = terrainId;
            }
        }
    }

    RedrawSelectedArea(currentTile.center, currentTile.centerTexture, minX, maxX, minY, maxY);
}

void MapEditor::RedrawCurrentTiles()
{
    if (currentTile.center == nullptr || currentTile.left == nullptr || currentTile.right == nullptr || currentTile.up == nullptr || currentTile.down == nullptr)
    {
        return;
    }
    
    // Determine the limits of the current tile.
    UpdateMinMaxHeights();

    RedrawSelectedArea(currentTile.center, currentTile.centerTexture, 0, tileSize - 1, 0, tileSize - 1);
    RedrawSelectedArea(currentTile.left, currentTile.leftTexture, 0, tileSize - 1, 0, tileSize - 1);
    RedrawSelectedArea(currentTile.right, currentTile.rightTexture, 0, tileSize - 1, 0, tileSize - 1);
    RedrawSelectedArea(currentTile.up, currentTile.topTexture, 0, tileSize - 1, 0, tileSize - 1);
    RedrawSelectedArea(currentTile.down, currentTile.bottomTexture, 0, tileSize - 1, 0, tileSize - 1);
}

void MapEditor::UpdateMinMaxHeights()
{
    currentTile.minHeight = std::numeric_limits<unsigned short>::max();
    currentTile.maxHeight = 0;
    for (int x = 0; x < tileSize; x++)
    {
        for (int y = 0; y < tileSize; y++)
        {
            int pos = (x + y * tileSize) * 4;
            unsigned short height = (unsigned short)currentTile.center[pos] + (((unsigned short)currentTile.center[pos + 1]) << 8);
            if (height > currentTile.maxHeight)
            {
                currentTile.maxHeight = height;
            }
            if (height < currentTile.minHeight)
            {
                currentTile.minHeight = height;
            }

            height = (unsigned short)currentTile.left[pos] + (((unsigned short)currentTile.left[pos + 1]) << 8);
            if (height > currentTile.maxHeight)
            {
                currentTile.maxHeight = height;
            }
            if (height < currentTile.minHeight)
            {
                currentTile.minHeight = height;
            }

            height = (unsigned short)currentTile.right[pos] + (((unsigned short)currentTile.right[pos + 1]) << 8);
            if (height > currentTile.maxHeight)
            {
                currentTile.maxHeight = height;
            }
            if (height < currentTile.minHeight)
            {
                currentTile.minHeight = height;
            }

            height = (unsigned short)currentTile.up[pos] + (((unsigned short)currentTile.up[pos + 1]) << 8);
            if (height > currentTile.maxHeight)
            {
                currentTile.maxHeight = height;
            }
            if (height < currentTile.minHeight)
            {
                currentTile.minHeight = height;
            }

            height = (unsigned short)currentTile.down[pos] + (((unsigned short)currentTile.down[pos + 1]) << 8);
            if (height > currentTile.maxHeight)
            {
                currentTile.maxHeight = height;
            }
            if (height < currentTile.minHeight)
            {
                currentTile.minHeight = height;
            }
        }
    }
}

void MapEditor::RedrawSelectedArea(unsigned char* tileData, sf::Texture& texture, int minX, int maxX, int minY, int maxY)
{
    // Merge in the raw data with our settings.
    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            // Rescale if requested.
            int pos = (x + y * tileSize) * 4;
            unsigned char rawValue;
            unsigned short height = (unsigned short)tileData[pos] + (((unsigned short)tileData[pos + 1]) << 8);
            if (displaySettings.rescale)
            {
                rawValue = (unsigned char)((float)(height - currentTile.minHeight) / (float)(currentTile.maxHeight - currentTile.minHeight) * 255);
            }
            else
            {
                rawValue = (unsigned char)(height / 256);
            }

            // Add contour lines if requested.
            bool contourLineHere = (displaySettings.showContours && (tileData[pos] == 1 || tileData[pos] == 0));

            convertedRawData[pos] = contourLineHere ? 255 : rawValue;
            convertedRawData[pos + 1] = contourLineHere ? 255 : rawValue;
            convertedRawData[pos + 2] = rawValue;
            convertedRawData[pos + 3] = 255;

            // Show the overlay if requested.
            if (displaySettings.showOverlay)
            {
                // empirically determined
                float overlayScale = 0.20f;
                sf::Color overlayColor = PaletteWindow::GetTerrainColor(PaletteWindow::GetNearestTerrainType(tileData[pos + 2]));
                convertedRawData[pos] = (unsigned short)((float)overlayColor.r * overlayScale) + (unsigned short)convertedRawData[pos] > 255 ? 255 : (unsigned char)((float)overlayColor.r * overlayScale) + convertedRawData[pos];
                convertedRawData[pos + 1] = (unsigned short)((float)overlayColor.g * overlayScale) + (unsigned short)convertedRawData[pos + 1] > 255 ? 255 : (unsigned char)((float)overlayColor.g * overlayScale) + convertedRawData[pos + 1];
                convertedRawData[pos + 2] = (unsigned short)((float)overlayColor.b * overlayScale) + (unsigned short)convertedRawData[pos + 2] > 255 ? 255 : (unsigned char)((float)overlayColor.b * overlayScale) + convertedRawData[pos + 2];
            }
        }
    }

    texture.update(convertedRawData);
}

void MapEditor::SaveTile()
{
    if (currentTile.center != nullptr)
    {
        summaryView.UpdateSelectedTile(currentTile.center);
    }
}

void MapEditor::Render(sf::RenderWindow& window)
{
    window.draw(currentTile.centerSprite);
    window.draw(currentTile.leftSprite);
    window.draw(currentTile.rightSprite);
    window.draw(currentTile.topSprite);
    window.draw(currentTile.bottomSprite);

    // Render our appropriate brush.
    sf::Color toolColor = PaletteWindow::GetTerrainColor(paletteWindow.GetTerrainType());
    toolColor.a = 60;
    if (paletteWindow.GetSelectedTool() == PaletteWindow::Tool::CIRCLE_BRUSH)
    {
        brushes.circleBrush.setRadius((float)paletteWindow.GetToolRadius());
        brushes.circleBrush.setFillColor(toolColor);
        window.draw(brushes.circleBrush);
    }
    else
    {
        brushes.squareBrush.setSize(sf::Vector2f(2.0f * (float)paletteWindow.GetToolRadius(), 2.0f * (float)paletteWindow.GetToolRadius()));
        brushes.squareBrush.setFillColor(toolColor);
        window.draw(brushes.squareBrush);
    }
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
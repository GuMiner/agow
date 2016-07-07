#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <stb/stb_image_write.h>
#include "ImageUtils.h"
#include "PaletteWindow.h"
#include "SummaryView.h"

SummaryView::SummaryView(int size, int tileCount, int reductionFactor)
    : isAlive(true), size(size), tileId(tileCount),
      topographicSummarizer(size, tileCount, reductionFactor, "../ContourTiler/rasters/summary/", "summary.png"),
      overlaySummarizer(size, tileCount, reductionFactor, "../ContourTiler/rasters/summary/", "overlay.png")
{

}

void SummaryView::MoveSelectedTile(Direction direction)
{
    int x, y;
    tileId.GetPositionFromId(selectedTile, &x, &y);

    switch (direction)
    {
    case UP: ++y; break;
    case DOWN: --y; break;
    case LEFT: --x; break;
    case RIGHT: ++x; break;
    }

    if (topographicSummarizer.IsTileValid(x, y))
    {
        selectedTile = tileId.GetTileId(x, y);
        UpdateSelectedTileRectangle();
    }
}

void SummaryView::LoadSelectedTile(unsigned char** rawData)
{
    if (*rawData != nullptr)
    {
        // We could also do topographic, but we (for now) don't support elevation changes with this program.

        ImageUtils::FreeImage(*rawData);
        *rawData = nullptr;
    }

    int x, y;
    tileId.GetPositionFromId(selectedTile, &x, &y);

    std::stringstream imageTile;
    imageTile << "../ContourTiler/rasters/" << y << "/" << x << ".png";

    int width, height;
    ImageUtils::LoadImage(imageTile.str().c_str(), &width, &height, rawData);
}

// We know that only the topographic data is what really matters. We do need to indicate a reload to both topographic and overlay layers though.
void SummaryView::UpdateSelectedTile(unsigned char* newData)
{
    int x, y;
    tileId.GetPositionFromId(selectedTile, &x, &y);
    topographicSummarizer.UpdateSummaryForTile(newData, x, y, true);
    overlaySummarizer.UpdateSummaryForTile(newData, x, y, false);
}

void SummaryView::UpdateSelectedTileRectangle()
{
    int motionScale = size / tileId.GetTileCount();
    int x, y;
    tileId.GetPositionFromId(selectedTile, &x, &y);
    selectedTileRectangle.setPosition(sf::Vector2f((float)(x * motionScale), (float)(size - (y + 1) * motionScale)));
}

void SummaryView::HandleEvents(sf::RenderWindow& window)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            // Ignore -- we only close if the main map editor closes.
        }
    }
}

void SummaryView::Render(sf::RenderWindow& window)
{
    window.draw(topographicSummarizer.GetSummarizedSprite());
    window.draw(overlaySummarizer.GetSummarizedSprite());
    window.draw(selectedTileRectangle);
}

void SummaryView::ThreadStart()
{
    topographicSummarizer.Initialize(
        [](unsigned char r, unsigned char g, unsigned char b, unsigned char a) -> float
        {
            return ((float)((unsigned short)r + (((unsigned short)g) << 8))) / (float)std::numeric_limits<unsigned short>::max();
        },
        [](float value, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a) -> void
        {
            unsigned short averageValue = (unsigned short)(value * (float)std::numeric_limits<unsigned short>::max());
            unsigned char actualValue = (unsigned char)(averageValue / 256);
            *r = actualValue;
            *g = actualValue;
            *b = actualValue;
            *a = 255;
        });

    overlaySummarizer.Initialize(
        [](unsigned char r, unsigned char g, unsigned char b, unsigned char a) -> float
        {
            return (float)b;
        },
        [](float value, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a) -> void
        {
            // This will probably look odd but it's the best I can do.
            unsigned char averageValue = (unsigned char)value;
            PaletteWindow::TerrainType type = PaletteWindow::GetNearestTerrainType(averageValue);
            sf::Color color = PaletteWindow::GetTerrainColor(type);
            *r = color.r;
            *g = color.g;
            *b = color.b;
            *a = 150;
        });

    selectedTileRectangle = sf::RectangleShape(sf::Vector2f((float)(size / tileId.GetTileCount()), (float)(size / tileId.GetTileCount())));
    selectedTileRectangle.setFillColor(sf::Color(0, 0, 0, 0));
    selectedTileRectangle.setOutlineThickness(1.0f);
    selectedTileRectangle.setOutlineColor(sf::Color::Green);
    UpdateSelectedTileRectangle();

    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style = sf::Style::Titlebar;
    sf::RenderWindow window(sf::VideoMode(size, size), "Summary", style, contextSettings);
    window.setFramerateLimit(60);

    // Start the main loop
    while (isAlive)
    {
        HandleEvents(window);
        Render(window);

        // Display what we rendered.
        window.display();
    }
}

void SummaryView::Start()
{
    executionThread = new std::thread(&SummaryView::ThreadStart, this);
}

void SummaryView::Stop()
{
    isAlive = false;
    executionThread->join();
    delete executionThread;
}
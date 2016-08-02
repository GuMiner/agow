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

SummaryView::SummaryView(int size, int tileCount, int offsetX, int offsetY, int visibleSideCount, int reductionFactor)
    : isAlive(false), size(size), tileId(tileCount), selectedTile(2955),
      topographicSummarizer(size, tileCount, offsetX, offsetY, visibleSideCount, reductionFactor, "../ContourTiler/rasters/summary/", "summary.png"),
      overlaySummarizer(size, tileCount, offsetX, offsetY, visibleSideCount, reductionFactor, "../ContourTiler/rasters/summary/", "overlay.png"),
      windowSelf(nullptr), offsetX(offsetX), offsetY(offsetY), visibleSideCount(visibleSideCount)
{
}

void SummaryView::RemapToTile(double* x, double* y)
{
    int tileX, tileY;
    tileId.GetPositionFromId(selectedTile, &tileX, &tileY);

    (*x) *= ((double)1000 * tileId.GetTileCount());
    (*y) *= ((double)1000 * tileId.GetTileCount());

    (*x) -= (tileX * 1000);
    (*y) -= (tileY * 1000);
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

        // TODO this doesn't apply on startup, and should be fixed to not pass around the window pointer.
        if (windowSelf != nullptr)
        {
            std::stringstream titleString;
            titleString << "Summary (" << x << ", " << y << ")" << std::endl;
            windowSelf->setTitle(titleString.str());
        }
    }
}

void SummaryView::LoadSelectedTile(unsigned char** data, int offsetX, int offsetY)
{
    if (*data != nullptr)
    {
        // We could also do topographic, but we (for now) don't support elevation changes with this program.

        ImageUtils::FreeImage(*data);
        *data = nullptr;
    }

    int x, y;
    tileId.GetPositionFromId(selectedTile, &x, &y);

    x += offsetX;
    y += offsetY;

    // Ensure we read a valid image.
    x = std::max(0, x);
    y = std::max(0, y);
    x = std::min(x, tileId.GetTileCount() - 1);
    y = std::min(y, tileId.GetTileCount() - 1);

    std::stringstream imageTile;
    imageTile << "../ContourTiler/rasters/" << y << "/" << x << ".png";

    int width, height;
    if (!ImageUtils::LoadImage(imageTile.str().c_str(), &width, &height, data))
    {
        std::cout << "Failed to load " << imageTile.str() << std::endl;
    }
}

void SummaryView::LoadSelectedTile(bool loadEdges, unsigned char** centerData, unsigned char** leftData, unsigned char** rightData, unsigned char** topData, unsigned char** bottomData)
{
    LoadSelectedTile(centerData, 0, 0);
    
    // Don't reload the edges if we aren't saving when moving to speed up drawing
    if (loadEdges)
    {
        LoadSelectedTile(leftData, -1, 0);
        LoadSelectedTile(rightData, 1, 0);
        LoadSelectedTile(topData, 0, 1);
        LoadSelectedTile(bottomData, 0, -1);
    }
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
    int motionScale = size / visibleSideCount;
    int x, y;
    tileId.GetPositionFromId(selectedTile, &x, &y);
    selectedTileRectangle.setPosition(sf::Vector2f((float)((x - offsetX) * motionScale), (float)(size - (y - offsetY + 1) * motionScale)));
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
            *a = 110;
        });

    selectedTileRectangle = sf::RectangleShape(sf::Vector2f((float)(size / visibleSideCount), (float)(size / visibleSideCount)));
    selectedTileRectangle.setFillColor(sf::Color(0, 0, 0, 0));
    selectedTileRectangle.setOutlineThickness(1.0f);
    selectedTileRectangle.setOutlineColor(sf::Color::Green);
    UpdateSelectedTileRectangle();

    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style = sf::Style::Titlebar;
    sf::RenderWindow window(sf::VideoMode(size, size), "Summary", style, contextSettings);
    window.setFramerateLimit(60);
    
    // TODO
    windowSelf = &window;

    // Start the main loop
    isAlive = true;
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
    while (!isAlive)
    {
        sf::sleep(sf::milliseconds(50));
    }
}

void SummaryView::Stop()
{
    isAlive = false;
    executionThread->join();
    delete executionThread;
}
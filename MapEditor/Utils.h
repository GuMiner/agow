#pragma once
#include <SFML\Graphics.hpp>

struct Line
{
    sf::Vector2f start;
    sf::Vector2f end;

    Line()
    {

    }

    Line(sf::Vector2f startPt, sf::Vector2f endPt)
    {
        start = startPt;
        end = endPt;
    }
};

class Utils
{
public:
    static bool LineIntersectsRectangle(sf::Vector2f start, sf::Vector2f end, 
        sf::Vector2f rectangleStart, sf::Vector2f rectangleSize);
    static bool LineIntersectsLine(Line first, Line second);
};


#include "Utils.h"

bool Utils::LineIntersectsRectangle(sf::Vector2f start, sf::Vector2f end, 
    sf::Vector2f rectangleStart, sf::Vector2f rectangleSize)
{
    sf::Vector2f rectangleEnd = rectangleStart + rectangleSize;

    // If the line is within the rectangle, return directly.
    if ((start.x > rectangleStart.x && start.x < rectangleEnd.x && start.y > rectangleStart.y && start.y < rectangleEnd.y) ||
        (end.x > rectangleStart.x && end.x < rectangleEnd.x && end.y > rectangleStart.y && end.y < rectangleEnd.y))
    {
        return true;
    }

    Line testLine(start, end);

    Line topEdge(rectangleStart, rectangleStart + sf::Vector2f(rectangleSize.x, 0));
    Line leftEdge(rectangleStart, rectangleStart + sf::Vector2f(0, rectangleSize.y));
    Line rightEdge(rectangleStart + sf::Vector2f(rectangleSize.x, 0), rectangleEnd);
    Line bottomEdge(rectangleStart + sf::Vector2f(0, rectangleSize.y), rectangleEnd);

    if (LineIntersectsLine(testLine, topEdge) || 
        LineIntersectsLine(testLine, leftEdge) || 
        LineIntersectsLine(testLine, rightEdge) || 
        LineIntersectsLine(testLine, bottomEdge))
    {
        return true;
    }

    return false;
}

bool Utils::LineIntersectsLine(Line first, Line second)
{
    sf::Vector2f firstVector = first.end - first.start;
    sf::Vector2f firstPerpVector = sf::Vector2f(firstVector.y, firstVector.x);
    sf::Vector2f secondVector = second.end - second.start;
    sf::Vector2f secondPerpVector = sf::Vector2f(secondVector.y, secondVector.x);

    float firstDotSecondPerp = firstVector.x * secondPerpVector.x + firstVector.y * secondPerpVector.y;
    if (firstDotSecondPerp == 0)
    {
        // Lines are parallel.
        return false;
    }

    sf::Vector2f startDiff = second.start - first.start;
    float t = (startDiff.x * secondPerpVector.x - startDiff.y * secondPerpVector.y) / firstDotSecondPerp;
    if (t > 0 && t < 1)
    {
        t = (startDiff.x * firstPerpVector.x - startDiff.y * firstPerpVector.y) / firstDotSecondPerp;
        if (t > 0 && t < 1)
        {
            return true;
        }
    }

    return false;
}
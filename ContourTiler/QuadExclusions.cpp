#include "QuadExclusions.h"

QuadExclusions::QuadExclusions()
    : exclusions()
{
}

bool QuadExclusions::ReadExclusions()
{
    // TODO
    return false;
}

bool QuadExclusions::IsExcluded(const sf::Vector2i& point) const
{
    return exclusions.find(point) != exclusions.end();
}

bool QuadExclusions::ToggleExclusion(const sf::Vector2i& point)
{
    std::set<sf::Vector2i>::iterator pointPos = exclusions.find(point);
    if (exclusions.find(point) != exclusions.end())
    {
        exclusions.erase(pointPos);
        return false;
    }
    else
    {
        exclusions.insert(point);
        return true;
    }
}

void QuadExclusions::WriteExclusions()
{
    // TODO
}
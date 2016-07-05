#include "TileIdentifier.h"



TileIdentifier::TileIdentifier(int tileCount)
    : tileCount(tileCount)
{

}
int TileIdentifier::GetTileId(int x, int y) const
{
    return x + y * tileCount;
}

void TileIdentifier::GetPositionFromId(int pos, int* x, int* y) const
{
    *x = pos % tileCount;
    *y = pos / tileCount;
}

int TileIdentifier::GetTileCount() const
{
    return tileCount;
}
#pragma once
class TileIdentifier
{
    int tileCount;

public:
    TileIdentifier(int tileCount);
    int GetTileId(int x, int y) const;
    void GetPositionFromId(int pos, int* x, int* y) const;
    int GetTileCount() const;
};


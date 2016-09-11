#pragma once
#include <string>
#include "Data\RenderableSentence.h"
#include "Managers\FontManager.h"
#include "Math\Vec.h"
#include "Utils\Vertex.h"

// Draws statistics information in a more fancier (and older) manner than using the GUI library.
//  This information is drawn with the FontManager global font.
class Statistics
{
    FontManager* fontManager;

    // Stats data
    int currentFrameCounter;
    float totalElapsedTime;
    vec::vec2i lastSector;

    int textPixelHeight;
    vec::mat4 textScale;

    // Overall Details
    RenderableSentence fps;
    RenderableSentence runTime;
    RenderableSentence sector;

public:
    Statistics();
    bool Initialize(FontManager* fontManager);

    void UpdateRunTime(float currentTime, float elapsedTime);
    void UpateSector(const vec::vec2i& sector);
    void RenderStats(vec::mat4& perspectiveMatrix);
};

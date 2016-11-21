#pragma once
#include <string>
#include <glm\vec3.hpp>
#include "Data\RenderableSentence.h"
#include "Managers\FontManager.h"
#include "Utils\Vertex.h"

// Draws statistics information in a more fancier (and older) manner than using the GUI library.
//  This information is drawn with the FontManager global font.
class Statistics
{
    FontManager* fontManager;

    // Stats data
    int currentFrameCounter;
    float totalElapsedTime;
    glm::ivec2 lastSector;

    int textPixelHeight;

    // Overall Details
    RenderableSentence fps;
    RenderableSentence runTime;
    RenderableSentence sector;

public:
    Statistics();
    bool Initialize(FontManager* fontManager);

    void UpdateRunTime(float currentTime, float elapsedTime);
    void UpateSector(const glm::ivec2& sector);
    void RenderStats(glm::mat4& perspectiveMatrix);
};

#pragma once
#include <string>
#include "Managers\FontManager.h"
#include "Math\Vec.h"
#include "Data\RenderableSentence.h"
#include "Utils\Vertex.h"

// Draws statistics information in a more fancier (and older) manner than using the GUI library.
//  This information is drawn with the FontManager global font.
class Statistics
{
    // Holds a reference to the font manager so we can update text sentences
    FontManager* fontManager;

    public:
        Statistics();
        bool Initialize(FontManager* fontManager);

        void UpdateRunTime(float currentTime, float elapsedTime);

        void RenderStats(vec::mat4& perspectiveMatrix);

    private:
        int currentFrameCounter;
        float totalElapsedTime;
        
        int textPixelHeight;
        vec::mat4 textScale;

        // Overall Details
        RenderableSentence fps;
        RenderableSentence runTime;
};

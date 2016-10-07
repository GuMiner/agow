#include <sstream>
#include <string>
#include <glm\gtc\matrix_transform.hpp>
#include "Statistics.h"

Statistics::Statistics()
    : textPixelHeight(20), textScale(glm::scale(glm::mat4(), glm::vec3(0.02f, 0.02f, 0.02f))), currentFrameCounter(0), totalElapsedTime(0.0f), lastSector(-1, -1)
{
    runTime.posRotMatrix = glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.821f, -0.221f, -1.0f)), glm::vec3(0.015f, 0.015f, 0.015f));
    runTime.color = glm::vec3(0.8f, 0.8f, 0.8f);

    fps.posRotMatrix = glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.821f, -0.121f, -1.0f)), glm::vec3(0.015f, 0.015f, 0.015f));
    fps.color = glm::vec3(0.0f, 1.0f, 0.0f);

    sector.posRotMatrix = glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.821f, -0.021f, -1.0f)), glm::vec3(0.015f, 0.015f, 0.015f));
    sector.color = glm::vec3(1.0f, 1.0f, 0.0f);
}

bool Statistics::Initialize(FontManager* fontManager)
{
    this->fontManager = fontManager;

    // Create the sentence objects to perform font manipulations on.
    runTime.sentenceId = fontManager->CreateNewSentence();
    fps.sentenceId = fontManager->CreateNewSentence();
    sector.sentenceId = fontManager->CreateNewSentence();

    return true;
}

void Statistics::UpateSector(const glm::ivec2& sector)
{
    if (sector.x != lastSector.x || sector.y != lastSector.y)
    {
        lastSector = sector;

        std::stringstream sectorString;
        sectorString << "[" << sector.x << ", " << sector.y << "]";
        fontManager->UpdateSentence(this->sector.sentenceId, sectorString.str(), textPixelHeight, this->sector.color);
    }
}

void Statistics::UpdateRunTime(float currentTime, float elapsedTime)
{
    ++currentFrameCounter;
    totalElapsedTime += elapsedTime;
    if (totalElapsedTime > 1.0f)
    {
        float fpsValue = (float)currentFrameCounter / totalElapsedTime;
        currentFrameCounter = 0;
        totalElapsedTime = 0;

        // Only update the streams once per second.
        std::stringstream textStream;

        int hours = (int)(currentTime / 3600);
        int minutes = (int)((currentTime - hours) / 60);
        int seconds = (int)currentTime - (3600 * hours + 60 * minutes);

        // Do some fancy stuff so values < 10 render similar to '00' instead of just '0'
        textStream << "Running for " <<
            (hours < 10 ? "0" : "") << hours << ":" <<
            (minutes < 10 ? "0" : "") << minutes << ":" <<
            (seconds < 10 ? "0" : "") << seconds;
        fontManager->UpdateSentence(runTime.sentenceId, textStream.str(), textPixelHeight, runTime.color);

        std::stringstream fpsStream;
        fpsStream.precision(2);
        fpsStream << std::fixed;
        fpsStream << "FPS: " << fpsValue;
        fontManager->UpdateSentence(fps.sentenceId, fpsStream.str(), textPixelHeight, fps.color);
    }
}

void Statistics::RenderStats(glm::mat4& perspectiveMatrix)
{
    fontManager->RenderSentence(runTime.sentenceId, perspectiveMatrix, runTime.posRotMatrix);
    fontManager->RenderSentence(fps.sentenceId, perspectiveMatrix, fps.posRotMatrix);
    fontManager->RenderSentence(sector.sentenceId, perspectiveMatrix, sector.posRotMatrix);
}

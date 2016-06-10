#include <sstream>
#include <string>
#include "Math\MatrixOps.h"
#include "Statistics.h"

Statistics::Statistics()
{
    textPixelHeight = 20;

    textScale = MatrixOps::Scale(0.02f, 0.02f, 0.02f);

    runTime.posRotMatrix = MatrixOps::Translate(-0.821f, -0.221f, -1.0f) * MatrixOps::Scale(0.015f, 0.015f, 0.015f);
    runTime.color = vec::vec3(0.8f, 0.8f, 0.8f);

    xPosition.posRotMatrix = MatrixOps::Translate(-0.821f, -0.321f, -1.0f) * textScale;
    yPosition.posRotMatrix = MatrixOps::Translate(-0.659f, -0.321f, -1.0f) * textScale;
    zPosition.posRotMatrix = MatrixOps::Translate(-0.508f, -0.321f, -1.0f) * textScale;

    xPosition.color = vec::vec3(1.0f, 0.0f, 0.0f);
    yPosition.color = vec::vec3(0.0f, 1.0f, 0.0f);
    zPosition.color = vec::vec3(0.0f, 0.0f, 1.0f);

    playerName.posRotMatrix = MatrixOps::Translate(-0.308f, -0.321f, -1.0f) * textScale * MatrixOps::Scale(2, 2, 2);
    playerName.color = vec::vec3(0.0f, 1.0f, 1.0f);
}

bool Statistics::Initialize(FontManager* fontManager)
{
    this->fontManager = fontManager;

    // Create the sentence objects to perform font manipulations on.
    playerCount.sentenceId = fontManager->CreateNewSentence();
    runTime.sentenceId = fontManager->CreateNewSentence();

    playerName.sentenceId = fontManager->CreateNewSentence();

    xPosition.sentenceId = fontManager->CreateNewSentence();
    yPosition.sentenceId = fontManager->CreateNewSentence();
    zPosition.sentenceId = fontManager->CreateNewSentence();

    return true;
}

void Statistics::UpdateRunTime(float currentTime)
{
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
}

void Statistics::UpdatePlayerDetails(std::string& playerName)
{
    // TODO
}

void Statistics::UpdateViewPos(const vec::vec3& position, const vec::mat4& viewMatrix)
{
    std::stringstream textStream;
    textStream.precision(2);
    textStream << std::fixed;

    textStream << "X: " << position.x;
    fontManager->UpdateSentence(xPosition.sentenceId, textStream.str(), textPixelHeight, xPosition.color);

    textStream.str("");
    textStream << "Y: " << position.y;
    fontManager->UpdateSentence(yPosition.sentenceId, textStream.str(), textPixelHeight, yPosition.color);

    textStream.str("");
    textStream << "Z: " << position.z;
    fontManager->UpdateSentence(zPosition.sentenceId, textStream.str(), textPixelHeight, zPosition.color);

    textStream.str("");
    textStream << " " << viewMatrix[0][0] << "," << viewMatrix[0][1] << "," << viewMatrix[0][2]
        << "|" << viewMatrix[1][0] << "," << viewMatrix[1][1] << "," << viewMatrix[1][2]
        << "|" << viewMatrix[2][0] << "," << viewMatrix[2][1] << "," << viewMatrix[2][2];
    fontManager->UpdateSentence(playerName.sentenceId, textStream.str(), textPixelHeight, playerName.color);

    
}

void Statistics::RenderStats(vec::mat4& perspectiveMatrix)
{
    fontManager->RenderSentence(playerCount.sentenceId, perspectiveMatrix, playerCount.posRotMatrix);
    fontManager->RenderSentence(runTime.sentenceId, perspectiveMatrix, runTime.posRotMatrix);

    fontManager->RenderSentence(playerName.sentenceId, perspectiveMatrix, playerName.posRotMatrix);

    fontManager->RenderSentence(xPosition.sentenceId, perspectiveMatrix, xPosition.posRotMatrix);
    fontManager->RenderSentence(yPosition.sentenceId, perspectiveMatrix, yPosition.posRotMatrix);
    fontManager->RenderSentence(zPosition.sentenceId, perspectiveMatrix, zPosition.posRotMatrix);
}

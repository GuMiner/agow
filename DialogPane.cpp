#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include "Math\PhysicsOps.h"
#include <sstream>
#include "Utils\Logger.h"
#include "DialogPane.h"

DialogPane::DialogPane()
    : isVisible(false)
{
}

bool DialogPane::LoadBasics(FontManager* fontManager, ShaderManager* shaderManager)
{
    this->fontManager = fontManager;
    
    /// Load in our shader for the dialog background
    if (!shaderManager->CreateShaderProgram("dialogRender", &programId))
    {
        Logger::LogError("Bad dialog shader!");
        return false;
    }

    // TODO configurable and not duplicated below.
    queuedPagesText.sentenceId = fontManager->CreateNewSentence();
    queuedPagesText.posRotMatrix = glm::translate(glm::mat4(), glm::vec3(0.529f, 0.25f, -1.0f)) * GetEffectScale(StyleText::Effect::NORMAL);
    queuedPagesText.color = glm::vec3(0.90f, 1.0f, 0.90f);

    glGenVertexArrays(1, &dialogVao);
    glBindVertexArray(dialogVao);

    return true;
}

// Adds new text for display.
void DialogPane::QueueText(const StyleText& newText)
{
    std::vector<StyleText> textToQueue;
    textToQueue.push_back(newText);
    QueueText(textToQueue);
}

void DialogPane::QueueText(const std::vector<StyleText>& newText)
{
    if (newText.size() == 0)
    {
        return;
    }

    dialogs.push_back(newText);
    if (!isVisible)
    {
        Advance();
    }

    isVisible = true;
    UpdateQueuedPages();
}

// TODO improve so that it trims on word boundaries.
void DialogPane::TrimToFit(StyleText text, std::vector<StyleText>* textLines)
{
    std::stringstream laggingSentence;
    std::stringstream sentence;

    for (unsigned int i = 0; i < text.text.length(); i++)
    {
        sentence << text.text[i];
        if (fontManager->SimulateSentenceLength(sentence.str(), DialogPane::PixelHeight) > DialogPane::MaxSentenceLength)
        {
            if (laggingSentence.str().length() != 0)
            {
                StyleText styleText = StyleText(text.speakerName, laggingSentence.str(), text.effect);
                styleText.color = text.color;
                textLines->push_back(styleText);
            }
            else
            {
                // Skip enormous characters.
                Logger::LogWarn("Skipped enormous character '", text.text[i], "' in '", text.text, "'.");
            }

            // Reset so that the lagging sentence is still lagging.
            laggingSentence.str("");
            sentence.str("");
            sentence << text.text[i];
        }

        laggingSentence << text.text[i];
    }

    // Add in any remainder.
    if (laggingSentence.str().length() != 0)
    {
        // Skip enormous characters.
        StyleText styleText = StyleText(text.speakerName, laggingSentence.str(), text.effect);
        styleText.color = text.color;
        textLines->push_back(styleText);
    }

    Logger::Log("Trimmed line into ", textLines->size(), " line(s).");
}

glm::mat4 DialogPane::GetEffectScale(StyleText::Effect effect)
{
    switch (effect)
    {
    case StyleText::MINI:
        return glm::scale(glm::mat4(), glm::vec3(0.016f));
    case StyleText::ITALICS:
        return glm::scale(glm::mat4(), glm::vec3(0.018f)) * PhysicsOps::Shear(0.0f, 1.0f);
    case StyleText::NORMAL:
    default:
        return glm::scale(glm::mat4(), glm::vec3(0.020f));
    }
}

void DialogPane::UpdateQueuedPages()
{
    // After all that, update the queued pages display.
    if (isVisible)
    {
        std::stringstream queuedPages;
        if (dialogs.size() == 0)
        {
            queuedPages << "@";
        }
        else
        {
            queuedPages << dialogs.size();
        }

        fontManager->UpdateSentence(queuedPagesText.sentenceId, queuedPages.str(), DialogPane::PixelHeight, queuedPagesText.color);
    }
}

// Moves to the next section of text, either paging or moving to the next set.
void DialogPane::Advance()
{
    // First, page text that's already been loaded.
    for (int i = 0; i < DialogPane::MaxLines && dialogText.size() != 0; i++)
    {
        fontManager->DeleteSentence(dialogText.front().sentenceId);
        dialogText.pop_front();

        fontManager->DeleteSentence(speakerNames.front().sentenceId);
        speakerNames.pop_front();
    }

    // Next, if there is no text left load in new text.
    if (dialogText.size() == 0 && dialogs.size() != 0)
    {
        const std::vector<StyleText>& styleText = dialogs.front();
        for (unsigned int i = 0; i < styleText.size(); i++)
        {
            if (styleText[i].text.length() == 0)
            {
                continue;
            }

            // Each line is broken into sublines if it is too long.
            std::vector<StyleText> sublines;
            TrimToFit(styleText[i], &sublines);
            for (unsigned int j = 0; j < sublines.size(); j++)
            {
                float yMin = 0.30f;

                // Create a proper sentence in the position for display, given that MaxLines are displayed at a time and we page in blocks.
                float yPos = ((float)((DialogPane::MaxLines - 1) - (j % DialogPane::MaxLines)) / (float)(DialogPane::MaxLines - 1)) * 0.12f + yMin;
                float xPos = 0.15f;
                RenderableSentence sentence;
                sentence.sentenceId = fontManager->CreateNewSentence();
                sentence.color = sublines[j].color;
                sentence.posRotMatrix = glm::translate(glm::mat4(), glm::vec3(xPos, yPos, -1.0f)) * GetEffectScale(sublines[j].effect);

                fontManager->UpdateSentence(sentence.sentenceId, sublines[j].text, DialogPane::PixelHeight, sentence.color);
                dialogText.push_back(sentence);

                RenderableSentence speakerSentence;
                speakerSentence.sentenceId = fontManager->CreateNewSentence();
                speakerSentence.color = glm::vec3(1.0f);
                speakerSentence.posRotMatrix = glm::translate(glm::mat4(), glm::vec3(xPos, 0.25f, -1.0f)) * GetEffectScale(StyleText::Effect::NORMAL);
                fontManager->UpdateSentence(speakerSentence.sentenceId, sublines[j].speakerName, DialogPane::PixelHeight, speakerSentence.color);
                speakerNames.push_back(speakerSentence);

                Logger::Log("Adding ", sublines[j].speakerName ,"'s phrase \"", sublines[j].text, "\", line ", j, ", [", xPos, ",", yPos, "].");
            }
        }
        
        dialogs.pop_front();
    }
    
    // Finally, if nothing is visible, turn off the display
    if (dialogText.size() == 0 && dialogs.size() == 0 && isVisible)
    {
        isVisible = false;
    }

    UpdateQueuedPages();
}

void DialogPane::Render(glm::mat4& perspectiveMatrix)
{
    if (isVisible)
    {
        // Render the dialog pane, which just draws a rectangle with stuffs.
        glUseProgram(programId);
        glBindVertexArray(dialogVao);
        glDrawArrays(GL_TRIANGLES, 0, 15);

        // Render the lines visible.
        int linesRendered = 0;
        for (auto iter = dialogText.cbegin(); iter != dialogText.cend() && linesRendered < DialogPane::MaxLines; iter++, linesRendered++)
        {
            fontManager->RenderSentence(iter->sentenceId, perspectiveMatrix, iter->posRotMatrix);
        }

        // Only render the first speaker name, they are all identical per-line. This may change in the future.
        fontManager->RenderSentence(speakerNames.cbegin()->sentenceId, perspectiveMatrix, speakerNames.cbegin()->posRotMatrix);

        fontManager->RenderSentence(queuedPagesText.sentenceId, perspectiveMatrix, queuedPagesText.posRotMatrix);
    }
}

void DialogPane::Callback(EventType eventType, void* callbackSpecificData)
{
    if (eventType == EventType::ADD_DIALOG && callbackSpecificData != nullptr)
    {
        DialogData* dialogData = (DialogData*)callbackSpecificData;
        QueueText(dialogData->dialogText);
    }
}

DialogPane::~DialogPane()
{
    glDeleteVertexArrays(1, &dialogVao);
}
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
    
    // TODO load graphics for dialog background shading.
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
                StyleText styleText;
                styleText.text = laggingSentence.str();
                styleText.color = text.color;
                styleText.effect = text.effect;
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
        StyleText styleText;
        styleText.text = laggingSentence.str();
        styleText.color = text.color;
        styleText.effect = text.effect;
        textLines->push_back(styleText);
    }

    Logger::Log("Trimmed line into ", textLines->size(), " line(s).");
}

// Moves to the next section of text, either paging or moving to the next set.
void DialogPane::Advance()
{
    // First, page text that's already been loaded.
    for (int i = 0; i < DialogPane::MaxLines && dialogText.size() != 0; i++)
    {
        fontManager->DeleteSentence(dialogText.front().sentenceId);
        dialogText.pop_front();
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
                // Create a proper sentence in the position for display, given that MaxLines are displayed at a time and we page in blocks.
                float yPos = ((float)((DialogPane::MaxLines - 1) - (j % DialogPane::MaxLines)) / (float)(DialogPane::MaxLines - 1)) * 0.60f;
                float xPos = 0.20f;
                RenderableSentence sentence;
                sentence.sentenceId = fontManager->CreateNewSentence();
                sentence.color = sublines[j].color;
                sentence.posRotMatrix = MatrixOps::Translate(-0.821f, -0.121f, -1.0f) * MatrixOps::Scale(0.015f, 0.015f, 0.015f);

                fontManager->UpdateSentence(sentence.sentenceId, sublines[j].text, DialogPane::PixelHeight, sentence.color);
                dialogText.push_back(sentence);

                Logger::Log("Adding phrase \"", sublines[j].text, "\", line ", j, ", [", xPos, ",", yPos, "].");
            }
        }
        
        dialogs.pop_front();
    }
    
    // Finally, if nothing is visible, turn off the display
    if (dialogText.size() == 0 && dialogs.size() == 0 && isVisible)
    {
        isVisible = false;
    }
}

void DialogPane::Render(vec::mat4& perspectiveMatrix)
{
    if (isVisible)
    {
        // Render the lines visible.
        int linesRendered = 0;
        for (auto iter = dialogText.crbegin(); iter != dialogText.crend() && linesRendered < DialogPane::MaxLines; iter++, linesRendered++)
        {
            fontManager->RenderSentence(iter->sentenceId, perspectiveMatrix, iter->posRotMatrix);
        }

        // TODO render dialog shader.
    }
}
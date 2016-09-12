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
    
    // TODO load graphics.
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
    isVisible = true;
}

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
                // Skip enormous characters.
                StyleText styleText;
                styleText.text = laggingSentence.str();
                styleText.color = text.color;
                styleText.effect = text.effect;
                textLines->push_back(styleText);
            }
            else
            {
                Logger::LogWarn("Skipped enormous character '", text.text[i], "' in '", text.text, "'.");
            }

            // Reset so that the lagging sentence is still lagging.
            laggingSentence.clear();
            sentence.clear();
            sentence << text.text[i];
            continue;
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
}

// Moves to the next section of text, either paging or moving to the next set.
void DialogPane::Advance()
{
    // First, page text that's already been loaded.
    if (dialogText.size() >= (unsigned int)DialogPane::MaxLines)
    {
        for (int i = 0; i < DialogPane::MaxLines && dialogText.size() != 0; i++)
        {
            fontManager->DeleteSentence(dialogText.front().sentenceId);
            dialogText.pop_front();
        }
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
                // TODO create a proper sentence in the proper position for display, given what we know about paging.
            }
        }
        
        dialogs.pop_front();
    }
    
    // Finally, if nothing is vislble, turn off the display/
    if (dialogText.size() == 0 && dialogs.size() == 0)
    {
        isVisible = false;
    }
}

void DialogPane::Render(vec::mat4& perspectiveMatrix)
{
    // Render the lines visible.
    int linesRendered = 0;
    for (auto iter = dialogText.crbegin(); iter != dialogText.crend() && linesRendered < DialogPane::MaxLines; iter++, linesRendered++)
    {
        fontManager->RenderSentence(iter->sentenceId, perspectiveMatrix, iter->posRotMatrix);
    }

    // TODO render dialog shader.
}
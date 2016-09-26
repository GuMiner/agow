#pragma once
#include <string>
#include <deque>
#include <vector>
#include <GL/glew.h>
#include "Data\RenderableSentence.h"
#include "Data\StyleText.h"
#include "Managers\FontManager.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

// Displays dialog text that NPCs speak.
class DialogPane
{
    const int PixelHeight = 20;
    const int MaxLines = 6;
    const float MaxSentenceLength = 25.0f; // on-screen viewpoint.
    FontManager* fontManager;

    std::deque<std::vector<StyleText>> dialogs;
    
    bool isVisible;

    // Note that the dialog rectangle is fixed and rendered by the shader itself.
    std::deque<RenderableSentence> dialogText;

    // Shader for rendering the dialog pane itself.
    GLuint programId;
    GLuint dialogVao;

    // Trims a single line of style text to fit within the max sentence length as multiple lines.
    void TrimToFit(StyleText text, std::vector<StyleText>* textLines);
    vec::mat4 GetEffectScale(StyleText::Effect effect);

public:
    DialogPane();
    ~DialogPane();
    bool LoadBasics(FontManager* fontManager, ShaderManager* shaderManager);

    // Adds new text for display.
    void QueueText(const StyleText& newText);
    void QueueText(const std::vector<StyleText>& newText);

    // Moves to the next section of text, either paging or moving to the next set.
    void Advance();

    void Render(vec::mat4& perspectiveMatrix);
};


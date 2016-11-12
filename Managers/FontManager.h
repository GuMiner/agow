#pragma once
#include <map>
#include <string>
#include <GL/glew.h>
#include <stb/stb_truetype.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include "Data\TextDataTypes.h"
#include "Utils\Vertex.h"
#include "shaders\ShaderFactory.h"

// Manages the in-game font.
class FontManager
{
    // Maps characters to the TextInfo representing each character,
    std::map<int, TextInfo> fontData;

    // Holds the font texture that is filled as necessary.
    GLuint fontTexture;
    int width;
    int height;

    int usedWidth;
    int usedHeight;
    int lastMaxHeight;

    // Holds our font shader information.
    GLuint fontShader;
    GLint projLocation, mvLocation;
    GLint fontImageLocation;

    // Holds STB font info for loading in new font data as necessary
    stbtt_fontinfo fontInfo;
    unsigned char *loadedFontFile;

    const int VerticesPerChar = 4;
    void AddToFontTexture(CharInfo& charInfo);
    CharInfo& GetCharacterInfo(int fontPixelHeight, int character);

    // Sentence information
    int nextSentenceId;
    std::map<int, SentenceInfo> sentences;

    int GetSentenceVertexCount(const std::string& sentence);
    universalVertices AllocateSentenceVertices(const std::string& sentence, int pixelHeight, glm::vec3 textColor, float* pixelSize);

    void ClearCharacterData(const SentenceInfo& sentenceInfo);
    void DeleteSentence(const SentenceInfo& sentenceInfo);

public:
    FontManager();
    bool LoadFont(ShaderFactory* shaderManager, const char* fontName);

    int CreateNewSentence();
    void UpdateSentence(int sentenceId, const std::string& sentence, int pixelHeight, glm::vec3 textColor);
    void RenderSentence(int sentenceId, const glm::mat4& perpective, const glm::mat4& mvMatrix);
    void DeleteSentence(int sentenceId);

    float SimulateSentenceLength(const std::string& sentence, int pixelHeight);

    ~FontManager();
};


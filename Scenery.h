#pragma once
#include <GL\glew.h>
#include "Managers\ModelManager.h"
#include "Managers\ShaderManager.h"
#include "Math\Vec.h"

// Holds static scenery objects that are rendered.
class Scenery
{
    public:
        Scenery(ModelManager* modelManager);

        bool Initialize(ShaderManager& shaderManager);
        void Render(vec::mat4& viewMatrix, vec::mat4& projectionMatrix);

        ~Scenery();

    private:

        // Sky cube image segments.
        unsigned char* xNegative;
        unsigned char* yPositive;
        unsigned char* xPositive;
        unsigned char* yNegative;
        unsigned char* zPositive;
        unsigned char* zNegative;

        unsigned char* rawImage;
        
        void CreateImageSegments(int imageWidth);
        void FreeImageSegments();

        // Ground plane
        vec::mat4 groundOrientation;
        unsigned int groundModelId;

        // Sky cube
        GLuint skyCubeProgram;
        GLuint viewMatrixLocation;
        GLuint skyCubeMapLocation;

        GLuint skyCubeVao;
        GLuint skyCubeTexture;

        ModelManager* modelManager;
};


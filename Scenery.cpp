#include <string>
#include <sstream>
#include "Utils\ImageUtils.h"
#include "Utils\Logger.h"
#include "Scenery.h"

Scenery::Scenery(ModelManager* modelManager, ImageManager* imageManager)
{
    this->modelManager = modelManager;
    this->imageManager = imageManager;

    groundOrientation = vec::mat4::identity();
}

bool Scenery::Initialize(ShaderManager& shaderManager)
{
    // Load the ground
    groundModelId = modelManager->LoadModel("models/scenery/ground");
    if (groundModelId == 0)
    {
        Logger::Log("Error loading the ground scenery model!");
        return false;
    }

    // Setup the sky program and sky image.

    // Sky program.
    if (!shaderManager.CreateShaderProgram("skyCubeRender", &skyCubeProgram))
    {
        Logger::Log("Failure creating the sky cube shader program!");
        return false;
    }

    viewMatrixLocation = glGetUniformLocation(skyCubeProgram, "viewMatrix");
    skyCubeMapLocation = glGetUniformLocation(skyCubeProgram, "skyCubeMap");
    colorTransformImageLocation = glGetUniformLocation(skyCubeProgram, "colorTransformImage");
    colorTransformOffsetLocation = glGetUniformLocation(skyCubeProgram, "colorTransformOffset");

    // Sky Image
    int width;
    int height;
    if (!ImageUtils::GetRawImage("images/scenery/sky.png", &rawImage, &width, &height))
    {
        return false;
    }

    if (height / 3 != width / 4)
    {
        Logger::Log("This is not a 4x3 image forming a potential cubemap.!");
        Logger::Log("Ensure your image width:height ratio is 4:3.");
        return false;
    }

    int imageWidth = height / 3;
    CreateImageSegments(imageWidth);

    // The sky image is stored as squares with height = 6 * width. Break and send each apart individually.
    glGenVertexArrays(1, &skyCubeVao);
    glBindVertexArray(skyCubeVao);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &skyCubeTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubeTexture);

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA, imageWidth, imageWidth);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, imageWidth, imageWidth, GL_RGBA, GL_UNSIGNED_BYTE, xPositive);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, imageWidth, imageWidth, GL_RGBA, GL_UNSIGNED_BYTE, xNegative);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, imageWidth, imageWidth, GL_RGBA, GL_UNSIGNED_BYTE, yPositive);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, imageWidth, imageWidth, GL_RGBA, GL_UNSIGNED_BYTE, yNegative);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, imageWidth, imageWidth, GL_RGBA, GL_UNSIGNED_BYTE, zPositive);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, imageWidth, imageWidth, GL_RGBA, GL_UNSIGNED_BYTE, zNegative);

    glActiveTexture(GL_TEXTURE1);
    colorTransformImageId = imageManager->CreateEmptyTexture(256, 256);
    imageManager->ResendToOpenGl(colorTransformImageId);

    return true;
}

void Scenery::Render(vec::mat4& viewMatrix, vec::mat4& projectionMatrix)
{
    // Render the ground plane
    modelManager->RenderModel(projectionMatrix, groundModelId, groundOrientation, false);

    // Render the sky
    glUseProgram(skyCubeProgram);
    glBindVertexArray(skyCubeVao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubeTexture);
    glUniform1i(skyCubeMapLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, imageManager->GetImage(colorTransformImageId).textureId);
    glUniform1i(colorTransformImageLocation, 1);

    glUniform2f(colorTransformOffsetLocation, 0.0f, 0.0f); // TODO animation is possible if this is moved.

    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, viewMatrix);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Scenery::CreateImageSegments(int imageWidth)
{
    xNegative = new unsigned char[imageWidth * imageWidth * 4];
    yPositive = new unsigned char[imageWidth * imageWidth * 4];
    xPositive = new unsigned char[imageWidth * imageWidth * 4];
    yNegative = new unsigned char[imageWidth * imageWidth * 4];
    zPositive = new unsigned char[imageWidth * imageWidth * 4];
    zNegative = new unsigned char[imageWidth * imageWidth * 4];
    
    // Ideally no rotations should be needed ... but I'm using a RH coordinate system which doesn't work well with the shader.
    ImageUtils::CopyImage(rawImage, xNegative, imageWidth * 4, 0, imageWidth, imageWidth, 1);
    ImageUtils::CopyImage(rawImage, yPositive, imageWidth * 4, imageWidth, imageWidth, imageWidth, 0);
    ImageUtils::CopyImage(rawImage, xPositive, imageWidth * 4, imageWidth * 2, imageWidth, imageWidth, 3);
    ImageUtils::CopyImage(rawImage, yNegative, imageWidth * 4, imageWidth * 3, imageWidth, imageWidth, 2);

    ImageUtils::CopyImage(rawImage, zNegative, imageWidth * 4, imageWidth, 0, imageWidth, 2);
    ImageUtils::CopyImage(rawImage, zPositive, imageWidth * 4, imageWidth, imageWidth * 2, imageWidth, 0);
}

void Scenery::FreeImageSegments()
{
    delete [] xNegative;
    delete [] yPositive;
    delete [] xPositive;
    delete [] yNegative;
    delete [] zPositive;
    delete [] zNegative;
}

Scenery::~Scenery()
{
    glDeleteVertexArrays(1, &skyCubeVao);
    glDeleteTextures(1, &skyCubeTexture);

    FreeImageSegments();
    ImageUtils::FreeRawImage(rawImage);
}

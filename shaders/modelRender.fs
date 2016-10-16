#version 400 core

uniform sampler2D modelTexture;
uniform sampler2D shadingColorAndFactor;

in vec2 uvPosition;
in flat int instanceId;

out vec4 color;

void main(void)
{
    // Scale each color of the provided object by the given color.
    const int size = 512;
    
    int xPos0 = (instanceId * 2) % size;
    int yPos0 = (instanceId * 2) / size;
    
    int xPos1 = (instanceId * 2 + 1) % size;
    int yPos1 = (instanceId * 2 + 1) / size;
    
    vec4 shadingColorDecomposed = texelFetch(shadingColorAndFactor, ivec2(xPos0, yPos0), 0);
    float selectionFactor = texelFetch(shadingColorAndFactor, ivec2(xPos1, yPos1), 0).r;
    
    color = (texture2D(modelTexture, vec2(uvPosition.x, 1.0f - uvPosition.y)) + 
        vec4(selectionFactor, selectionFactor, selectionFactor, 0.0f)) * shadingColorDecomposed;
}

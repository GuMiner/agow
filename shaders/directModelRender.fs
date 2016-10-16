#version 400 core

uniform sampler2D modelTexture;
uniform vec4 shadingColor;
uniform float selectionFactor;

in vec2 uvPosition;
out vec4 color;
 
void main(void)
{
    // Scale each color of the provided object by the given color.
    color = (texture2D(modelTexture, vec2(uvPosition.x, 1.0f - uvPosition.y)) + vec4(selectionFactor, selectionFactor, selectionFactor, 0.0f)) * shadingColor;
}
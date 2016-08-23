#version 400 core

uniform sampler2D modelTexture;
uniform vec4 shadingColor;
uniform float selectionFactor;

out vec4 color;

in vec2 uvPosition;

void main(void)
{
    // Scale each color of the provided object by the given color.
    color = (texture2D(modelTexture, uvPosition) + vec4(selectionFactor, selectionFactor, selectionFactor, 0.0f)) * shadingColor;
}

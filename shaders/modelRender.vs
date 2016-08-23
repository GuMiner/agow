#version 400

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uvPos;

out vec2 uvPosition;

uniform mat4 mvMatrix;
uniform mat4 projMatrix;

// Perform our position and projection transformations, and pass-through the color / texture data
void main(void)
{
    uvPosition = uvPos;
    gl_Position = projMatrix * mvMatrix * vec4(position, 1);
}
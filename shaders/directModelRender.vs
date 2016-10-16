#version 400 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uvPos;

out vec2 uvPosition;
uniform mat4 mvMatrix;
uniform mat4 projMatrix;

// Performs direct model rendering for an item. Good for rapidly-changing, < 100 items.
void main(void)
{
    uvPosition = uvPos;
    gl_Position = projMatrix * mvMatrix * vec4(position, 1);
}
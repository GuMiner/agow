#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 4) in uint id; // The ID represents the line thickness

out vec3 fs_color;
out flat uint lineThickness;
out vec2 linePos;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

// Renders tree trunks.
void main(void)
{
    lineThickness = id;
	fs_color = color;
    
    vec4 position = projMatrix * mvMatrix * vec4(position.x, position.y, position.z, 1.0f);
    linePos = position.xy;
    gl_Position = position;
}

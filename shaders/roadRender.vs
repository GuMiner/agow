#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec4 fs_color;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

// Renders travellers in ROAD areas.
void main(void)
{
	fs_color = vec4(color, 1.0f);
    gl_Position = projMatrix * mvMatrix * vec4(position, 1.0f);
}

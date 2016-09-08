#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 fs_color;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

// Renders tree trunks.
void main(void)
{
	fs_color = color;
    gl_Position = projMatrix * mvMatrix * vec4(position.x, position.y, position.z, 1.0f);
}

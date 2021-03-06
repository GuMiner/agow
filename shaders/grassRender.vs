#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec4 gs_color;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

// Renders grass stalks for GRASSLAND areas.
void main(void)
{
	gs_color = vec4(color, 0.8f);
    gl_Position = projMatrix * mvMatrix * vec4(position.x, position.y, position.z, 1.0f);
}

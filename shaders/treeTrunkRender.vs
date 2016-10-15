#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 4) in uint size;

out vec3 fs_color;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

// Renders tree trunks.
void main(void)
{
    // TODO scale the size based on uniforms to give a nice-sized tree trunk w/ branches.
	fs_color = color * sin(size);
    gl_Position = projMatrix * mvMatrix * vec4(position.x, position.y, position.z, 1.0f);
}

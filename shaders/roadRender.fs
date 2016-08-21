#version 400 core

out vec4 color;
in vec4 fs_color;

void main(void)
{
	color = fs_color;
}
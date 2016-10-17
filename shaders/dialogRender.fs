#version 400 core

in flat int stage;
out vec4 color;

void main(void)
{
    if (stage == 0)
    {
        color = vec4(0.20f, 0.20f, 0.20f, 0.8f);
    }
    else if (stage == 1)
    {
        color = vec4(0.30f, 0.30f, 0.30f, 0.8f);
    }
    else if (stage == 2)
    {
        color = vec4(0.20f, 0.20f, 0.30f, 0.8f);
    }
}
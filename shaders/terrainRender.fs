#version 400 core

// Use the terrain texture for fragment shading. TODO this eventually will be a lot more complicated / interesting and use the other color components.
uniform sampler2D terrainTexture;

in TES_OUT
{
    vec2 tc;
    float vertexId;
} fs_in;

out vec4 color;

void main(void)
{
    // Taken from our test tile, gives a good gradient to test tesselation with.
    const float min = 0.302;
    const float max = 0.420;
    
    if (true) // fract(fs_in.vertexId) < 0.2)
    {
        color = vec4(vec3(((texture(terrainTexture, fs_in.tc).r - min) / (max - min)) * 0.6 + 0.2), 1.0);
    }
    else
    {
        color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

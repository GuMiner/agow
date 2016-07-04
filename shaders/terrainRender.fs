#version 400 core

// Use the terrain texture for fragment shading. TODO this eventually will be a lot more complicated / interesting and use the other color components.
uniform sampler2D terrainTexture;

flat in TES_OUT
{
    smooth vec2 tc;
    flat float scaleFactor;
} fs_in;

out vec4 color;

void main(void)
{
    // Taken from our test tile, gives a good gradient to test tesselation with.
    const float min = 0.155;
    const float max = 0.173;
    
    float height = texture(terrainTexture, fs_in.tc).r;
    if (int(height * 10000) % 10 == 0)
    {
        color = vec4(0.2f, 0.8f, 0.2f, 1.0f);
    }
    else
    {
        color = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    }
    
    color = vec4(vec3(color) * fs_in.scaleFactor, 1.0);
    
    /*
    // Apply lighting calculations.
    color = vec4(0.5f, 0.5f, 0.5f, 1.0f); 
    color *= fs_in.scaleFactor;
    color.w = 1.0;
    
    */
    /*
    if (true) // fract(fs_in.vertexId) < 0.2)
    {
        color = vec4(vec3((( - min) / (max - min)) * 0.6 + 0.2), 1.0);
    }
    else
    {
        color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }*/
}

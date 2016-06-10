#version 400 core

uniform samplerCube skyCubeMap;

out vec4 color;

in VS_OUT
{
    vec3 uvwPos;
} fs_in;

void main(void)
{
    // Simply apply the color taking it from the cube map.
    vec4 textureColor = texture(skyCubeMap, fs_in.uvwPos);
    
    if (textureColor.x > 0.20)
    {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        color = textureColor;
    }
}
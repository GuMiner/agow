#version 400 core

uniform samplerCube skyCubeMap;

// 512 entries.
uniform sampler2D colorTransformImage;
uniform vec2 colorTransformOffset; 

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
        vec2 uvPos = vec2((fs_in.uvwPos.x + 1.0) / 2.0, (fs_in.uvwPos.y + 1.0) / 2.0);
        color = texture(colorTransformImage, uvPos + colorTransformOffset);
    }
    else
    {
        color = textureColor;
    }
}
#version 400 core

uniform samplerCube skyCubeMap;

in vec3 uvwPos;

// 512 entries.
uniform sampler2D colorTransformImage;
uniform vec2 colorTransformOffset; 

out vec4 color;

void main(void)
{
    // Simply apply the color taking it from the cube map.
    vec4 textureColor = texture(skyCubeMap, uvwPos);
    
    if (textureColor.x > 0.20)
    {
        vec2 uvPos = vec2((uvwPos.x + 1.0) / 2.0, (uvwPos.y + 1.0) / 2.0);
        color = texture(colorTransformImage, uvPos + colorTransformOffset);
    }
    else
    {
        color = textureColor;
    }
	
	color = vec4((int(uvwPos.x * 200) % 20) / 20.0f, (int(uvwPos.y * 200) % 20) / 20.0f, (int(uvwPos.z * 200) % 20) / 20.0f, 1.0f);
}
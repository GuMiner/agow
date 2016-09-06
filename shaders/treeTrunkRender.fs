#version 400 core

out vec4 color;

in vec3 fs_color;
in flat uint lineThickness;
in vec2 linePos;

void main(void)
{
    // TODO fix this to actually interpolate positions.
    vec2 convFragCoord = vec2(gl_FragCoord.x, -gl_FragCoord.y) * 2.0f;
    float diff = 
        (convFragCoord.x - linePos.x) * (convFragCoord.x - linePos.x) + 
        (convFragCoord.y - linePos.y) * (convFragCoord.y - linePos.y);
    
    // Fade out the color using the inverse of the difference.
    diff += 1.0f;
    
	color = vec4(fs_color, 1000000.0 / (diff * 2.0f));
}
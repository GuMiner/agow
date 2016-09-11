#version 400 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec4 fs_color;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

const float constAtten  = 0.9;
const float linearAtten = 0.6;
const float quadAtten   = 0.01;

// Renders tree leaves.
void main(void)
{
	fs_color = vec4(color, 0.66f);
    
    vec4 eyePos = mvMatrix * vec4(position.x, position.y, position.z, 1.0f);
    float dist = distance(eyePos, vec4(0.0f, 0.0f, 0.0f, 1.0f));
    eyePos.z += 0.1f;
    
    float attenuation = inversesqrt(constAtten + linearAtten * dist + quadAtten * dist * dist);
    gl_PointSize = 60.0f * attenuation;
    gl_Position = projMatrix * eyePos;
}

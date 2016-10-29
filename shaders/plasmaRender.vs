#version 400

layout (location = 0) in vec3 vertexPosition;

uniform float frameTime;

uniform vec3 position;
uniform mat4 projMatrix;

out vec3 uvwPos;

// Renders a plasma ball.
// FYI, via performance analysis it is much more efficent to 
//  pass this in via an array rather than hardcoding it into the shader.
void main(void)
{
    gl_Position = projMatrix * vec4(position + vertexPosition * (0.90f + (cos(frameTime) + 1.0f) * 0.10f), 1.0f);
    uvwPos = vertexPosition;
}

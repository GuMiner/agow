#version 400 core

layout (lines) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

in vec4 gs_color [];
out vec4 fs_color;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

// Renders grass stalks for GRASSLAND areas.
void main(void)
{
    vec4 startingPos = gl_in[0].gl_Position;
    
    // Line positions stay the same.
    for (int i = 0; i < gl_in.length(); i++)
    {
        gl_Position = gl_in[i].gl_Position;
        fs_color = gs_color[i];
        
        EmitVertex();
    }
    
    // Position the last-end pseudo-randomly, forming a point.
    gl_Position = startingPos + vec4(sin(startingPos.x / 10.0f), cos(startingPos.z / 20.0f), 0.0f, 0.0f);
    fs_color = gs_color[0];
    EmitVertex();

    EndPrimitive();
}

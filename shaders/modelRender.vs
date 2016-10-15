#version 400

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uvPos;

out vec2 uvPosition;
out flat int instanceId;

uniform sampler2D mvMatrix;

uniform mat4 projMatrix;

// Perform our position and projection transformations, and pass-through the color / texture data
void main(void)
{
    const int size = 256;
    
    // Extract out the MV matrix given our current instance.
    int xPos0 = (gl_InstanceID * 4) % size;
    int yPos0 = (gl_InstanceID * 4) / size;
    
    int xPos1 = (gl_InstanceID * 4 + 1) % size;
    int yPos1 = (gl_InstanceID * 4 + 1) / size;
    
    int xPos2 = (gl_InstanceID * 4 + 2) % size;
    int yPos2 = (gl_InstanceID * 4 + 2) / size;
    
    int xPos3 = (gl_InstanceID * 4 + 3) % size;
    int yPos3 = (gl_InstanceID * 4 + 3) / size;
    
    mat4 modelViewMatrix;
    modelViewMatrix[0] = texelFetch(mvMatrix, ivec2(xPos0, yPos0), 0);
    modelViewMatrix[1] = texelFetch(mvMatrix, ivec2(xPos1, yPos1), 0);
    modelViewMatrix[2] = texelFetch(mvMatrix, ivec2(xPos2, yPos2), 0);
    modelViewMatrix[3] = texelFetch(mvMatrix, ivec2(xPos3, yPos3), 0);
     
    uvPosition = uvPos;
    instanceId = gl_InstanceID;
    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1);
}
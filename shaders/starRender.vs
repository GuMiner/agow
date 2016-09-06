#version 400

uniform mat4 projMatrix;

out vec3 uvwPos;

void main(void)
{
    // We hardcode a cube here, to avoid doing matrix inversions, etc.
    const vec3[8] vertices = vec3[8](
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0,  1.0));
    
    // bottom, top, left, right, front, and back.
    const int[36] indices = int[36](
        0, 1, 2, 1, 2, 3,
        4, 5, 6, 5, 6, 7,
        0, 2, 6, 0, 6, 4,
        1, 3, 7, 1, 7, 5,
        0, 1, 5, 0, 5, 4,
        2, 3, 7, 2, 7, 6);
        
    float distance = 10000.0f;
    
    int index = indices[gl_VertexID];
    vec3 vertex = vertices[index];
    gl_Position = projMatrix * vec4((vertex * distance), 1.0f);
    uvwPos = vertex;
}

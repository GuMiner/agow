#version 400

void main(void)
{
    vec3[4] vertices = vec3[4](
        vec3(0.25, 0.55, 0.9),
        vec3(1.0,  0.55, 0.9),
        vec3(0.25, 1.0, 0.9),
        vec3(1.0,  1.0, 0.9));

    // TODO pass along some data to make a gradient in the FS.
    gl_Position = vec4(vertices[gl_VertexID], 1.0);
}
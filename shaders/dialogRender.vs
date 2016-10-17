#version 400

out flat int stage;

void main(void)
{
    vec3[15] vertices = vec3[15](
        vec3(0.15, 0.60, 0.9), // Upper box
        vec3(1.0,  0.60, 0.9),
        vec3(1.0,  1.0, 0.9),
        vec3(1.0,  1.0, 0.9),
        vec3(0.15, 1.0, 0.9),
        vec3(0.15, 0.60, 0.9),
        vec3(0.15, 0.60, 0.9), // Name box
        vec3(0.55, 0.60, 0.9),
        vec3(0.55, 0.50, 0.9),
        vec3(0.15, 0.50, 0.9),
        vec3(0.15, 0.60, 0.9),
        vec3(0.55, 0.50, 0.9),
        vec3(0.60, 0.60, 0.9), // Number of speakers pending.
        vec3(0.75, 0.60, 0.9),
        vec3(0.65, 0.50, 0.9));

    // TODO pass along some data to make a gradient in the FS.
    stage = gl_VertexID < 12 ? (gl_VertexID < 6 ? 0 : 1) : 2;
    gl_Position = vec4(vertices[gl_VertexID], 1.0);
}
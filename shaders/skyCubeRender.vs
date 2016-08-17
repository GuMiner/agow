#version 400

out VS_OUT
{
    vec3 uvwPos;
} vs_out;

uniform mat4 viewMatrix;
uniform mat4 perspectiveMatrix;

// Multiply are fixed vertices by the view matrix to render the full-screen sky cube.
void main(void)
{
    vec3[4] vertices = vec3[4](
        vec3(-1.0, -1.0, 1.0),
        vec3(1.0, -1.0, 1.0),
        vec3(-1.0, 1.0, 1.0),
        vec3(1.0, 1.0, 1.0));

    // OpenGL is a LH system, so flip the matrix appropriately.
    mat3 rhMatrix = transpose(mat3(viewMatrix));
    vec3 uvwPos = rhMatrix * vec3(-vertices[gl_VertexID].xy, vertices[gl_VertexID].z);
    
    // This algorithm unfortunately maps the cube as a cube, leading to distortions in moving.
    // To accommodate that, we figure out the current angle and distort the z-axis.
    vec3 upVector = vec3(0, 0, 1);
    vec3 givenVector = rhMatrix * upVector;
    
    float scaleFactor = dot(upVector, normalize(givenVector));
    uvwPos.z -= pow(1.0 - scaleFactor, 2) * 0.20;
    
    vs_out.uvwPos = vec3(-uvwPos.xy, uvwPos.z);
    gl_Position = vec4(vertices[gl_VertexID].xy, 1.0, 1.0);
}

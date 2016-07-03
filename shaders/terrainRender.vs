#version 400 core

out VS_OUT
{
    vec2 tc;
} vs_out;

void main(void)
{
    // Hardcode our vertex data so we don't need to send it into the program.
    const float terrainCellHalfSize = 0.5;
    const vec4 vertices[] = vec4[](
        vec4(-terrainCellHalfSize, 0.0, -terrainCellHalfSize, 1.0),
        vec4( terrainCellHalfSize, 0.0, -terrainCellHalfSize, 1.0),
        vec4(-terrainCellHalfSize, 0.0,  terrainCellHalfSize, 1.0),
        vec4( terrainCellHalfSize, 0.0,  terrainCellHalfSize, 1.0));
    
    const int textureSize = 1000;
    int x = gl_InstanceID % textureSize;
    int y = gl_InstanceID / textureSize;
    vec2 offset = vec2(x, y);
    
    // Figure out the texture coordinate by taking the position on the height texture and then the position in the cell.
    vs_out.tc = offset / float(textureSize) + ((vertices[gl_VertexID].xz + vec2(terrainCellHalfSize)) / (2 * terrainCellHalfSize)) / float(textureSize);
    
    // Move the terrain cells appropriately.
    gl_Position = vertices[gl_VertexID] + vec4(2 * terrainCellHalfSize * float(x - textureSize / 2), 0.0, 2 * terrainCellHalfSize * float(y - textureSize / 2), 0.0);
}

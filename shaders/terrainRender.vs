#version 400 core

out VS_OUT
{
    vec2 tc;
    float vertexId;
} vs_out;

void main(void)
{
    // This gives a cell 1m x 1m in size, which is ~3ft x 3ft, which is the export from our program.
    const float terrainCellHalfSize = 0.5;
    
    // Hardcode our vertex data so we don't need to send it into the program.
    const vec4 vertices[] = vec4[](
        vec4(-terrainCellHalfSize, -terrainCellHalfSize, 0.0, 1.0),
        vec4( terrainCellHalfSize, -terrainCellHalfSize, 0.0, 1.0),
        vec4(-terrainCellHalfSize,  terrainCellHalfSize, 0.0, 1.0),
        vec4( terrainCellHalfSize,  terrainCellHalfSize, 0.0, 1.0));
    
    // TODO this likely will need to be smaller to more cleanly load in terrain as needed / unload afterwords.
    const int textureSize = 1000;
    int x = gl_InstanceID % textureSize;
    int y = gl_InstanceID / textureSize;
    vec2 offset = vec2(x, y);
    
    // Figure out the texture coordinate by taking the position on the height texture and then the position in the cell.
    vs_out.tc = (offset + (vertices[gl_VertexID].xy + vec2(terrainCellHalfSize)) / (2 * terrainCellHalfSize)) / float(textureSize);
    
    // Used for wireframe effects.
    vs_out.vertexId = gl_VertexID;
    
    // Move the terrain cells appropriately.
    gl_Position = vertices[gl_VertexID] + vec4(2 * terrainCellHalfSize * float(x), 2 * terrainCellHalfSize * float(y), 0.0, 0.0);
}

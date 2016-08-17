#version 400 core

out VS_OUT
{
    vec2 tc;
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
    
    // TODO make this 100 to deal with subdivided regions.
    const int textureSize = 1000;
    int x = gl_InstanceID % textureSize;
    int y = gl_InstanceID / textureSize;
    vec2 offset = vec2(x, y);
    
    // Figure out the texture coordinate by taking the position on the height texture and then the position in the cell.
	// TODO for each image, add a pixel on both sides to avoid seam errors.
    vs_out.tc = (offset + (vertices[gl_VertexID].xy + vec2(terrainCellHalfSize)) / (2 * terrainCellHalfSize)) / float(textureSize);
    
    // Move the terrain cells appropriately.
    gl_Position = vertices[gl_VertexID] + vec4(2 * terrainCellHalfSize * float(x), 2 * terrainCellHalfSize * float(y), 0.0, 0.0);
}

#version 400 core

out vec2 tcs_in;

void main(void)
{
    // A cell 1m x 1m in size, gives us ~3ft x 3ft, which is the export from our program.
    const float terrainCellHalfSize = 0.5;
	
    // Hardcode our vertex data so we don't need to send it into the program.
    const vec4 vertices[] = vec4[](
        vec4(-terrainCellHalfSize, -terrainCellHalfSize, 0.0, 1.0),
        vec4( terrainCellHalfSize, -terrainCellHalfSize, 0.0, 1.0),
        vec4(-terrainCellHalfSize,  terrainCellHalfSize, 0.0, 1.0),
        vec4( terrainCellHalfSize,  terrainCellHalfSize, 0.0, 1.0));
    
    // This is 100, not 1k, because the regions are subdivided.
    const int textureSize = 100;
    int x = gl_InstanceID % textureSize;
    int y = gl_InstanceID / textureSize;
    vec2 offset = vec2(x, y);
    
    // Figure out the texture coordinate by taking the position on the height texture and then the position in the cell.
	// We add a pixel upon division to avoid seam errors.
    tcs_in = (offset + (vertices[gl_VertexID].xy + vec2(terrainCellHalfSize)) / (2 * terrainCellHalfSize)) / float(textureSize + 1);
    
    // Move the terrain cells appropriately.
    gl_Position = vertices[gl_VertexID] + vec4(2 * terrainCellHalfSize * float(x), 2 * terrainCellHalfSize * float(y), 0.0, 0.0);
}

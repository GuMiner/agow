#version 400 core

out VS_OUT
{
    vec2 tc;
} vs_out;

void main(void)
{
    // A cell 1m x 1m in size, gives us ~3ft x 3ft, which is the export from our program.
    
    // Hardcode our vertex data so we don't need to send it into the program.
    const vec4 vertices[] = vec4[](
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(1.0, 1.0, 0.0, 1.0));
    
    // This is 100, not 1k, because the regions are subdivided.
    const int textureSize = 100;
    int x = gl_InstanceID % textureSize;
    int y = gl_InstanceID / textureSize;
    vec2 offset = vec2(x, y);
    
    // Figure out the texture coordinate by taking the position on the height texture and then the position in the cell.
	// We add a pixel upon division to avoid seam errors.
    vs_out.tc = (offset + (vertices[gl_VertexID].xy)) / float(textureSize + 2);
    
    // Move the terrain cells appropriately.
    gl_Position = vertices[gl_VertexID] + vec4(float(x), float(y), 0.0, 0.0);
}

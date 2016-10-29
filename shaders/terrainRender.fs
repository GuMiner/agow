#version 400 core

// Use the terrain texture for fragment shading. TODO this eventually will be a lot more complicated / interesting and use the other color components.
uniform sampler2D terrainTexture;
uniform sampler2D terrainType;

uniform float gameTime;

smooth in vec2 tc_fs;
smooth in vec3 fragment_position;
flat in float scaleFactor;

out vec4 color;

// Copied from PaletteWindow.cpp, MapEditor.
const int SNOW_PEAK = 0;
const int ROCKS = 25;
const int TREES = 50;
const int DIRTLAND = 75;
const int GRASSLAND = 100;
const int ROADS = 125;
const int CITY = 150;
const int SAND = 175;
const int RIVER = 200;
const int LAKE = 225;

const vec4 SNOW_PEAK_COLOR = vec4(245.0f / 255.0f,  244.0f / 255.0f,  253.0f / 255.0f, 1.0f);
const vec4 ROCKS_COLOR = vec4(140.0f / 255.0f,  115.0f / 255.0f,  115.0f / 255.0f, 1.0f);
const vec4 TREES_COLOR = vec4(22.0f / 255.0f,  137.0f / 255.0f,  10.0f / 255.0f, 1.0f);
const vec4 DIRTLAND_COLOR = vec4(185.0f / 255.0f,  122.0f / 255.0f,  87.0f / 255.0f, 1.0f);
const vec4 GRASSLAND_COLOR = vec4(122.0f / 255.0f,  243.0f / 255.0f,  129.0f / 255.0f, 1.0f);
const vec4 ROADS_COLOR = vec4(192.0f / 255.0f,  192.0f / 255.0f,  192.0f / 255.0f, 1.0f);
const vec4 CITY_COLOR = vec4(170.0f / 255.0f,  34.0f / 255.0f,  181.0f / 255.0f, 1.0f);
const vec4 SAND_COLOR = vec4(255.0f / 255.0f,  255.0f / 255.0f,  128.0f / 255.0f, 1.0f);
const vec4 RIVER_COLOR = vec4(121.0f / 255.0f,  121.0f / 255.0f,  255.0f / 255.0f, 1.0f);
const vec4 LAKE_COLOR = vec4(0.0f / 255.0f,  0.0f / 255.0f,  179.0f / 255.0f, 1.0f);

vec4 GetTypeColor(int type)
{
	switch (type)
	{
		case SNOW_PEAK: return SNOW_PEAK_COLOR;
		case ROCKS: return ROCKS_COLOR;
		case TREES: return TREES_COLOR;
		case DIRTLAND: return DIRTLAND_COLOR;
		case GRASSLAND: return GRASSLAND_COLOR;
		case ROADS: return ROADS_COLOR;
		case CITY: return CITY_COLOR;
		case SAND: return SAND_COLOR;
		case RIVER: return RIVER_COLOR;
		case LAKE: return LAKE_COLOR;
	}
}

int GetNearestType(int type)	
{
	if (type < LAKE)
    {
        if (type < RIVER)
        {
            if (type < SAND)
            {
                if (type < CITY)
                {
                    if (type < ROADS)
                    {
                        if (type < GRASSLAND)
                        {
                            if (type < DIRTLAND)
                            {
                                if (type < TREES)
                                {
                                    if (type < ROCKS)
                                    {
										return SNOW_PEAK;
                                    }

									return ROCKS;
                                }

								return TREES;
                            }

							return DIRTLAND;
                        }

						return GRASSLAND;
                    }

					return ROADS;
                }

				return CITY;
            }

			return SAND;
        }

		return RIVER;
    }
	
	return LAKE;
}

void main(void)
{
    // Taken from our test tile, gives a good gradient to test tesselation with.
    const float min = 0.155;
    const float max = 0.173;
    
    float height = texture(terrainTexture, tc_fs).r;
    if (int(height * 10000) % 10 == 1)
    {
        color = vec4(0.4f, 0.45f, 0.4f, 1.0f);
    }
    else
    {
        color = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    }
    
	int type = GetNearestType(int(texelFetch(terrainType, ivec2(tc_fs.x * 100 + 1, tc_fs.y * 100 + 1), 0).r * 255.0f));
	vec4 typeColor = GetTypeColor(type);
	
	if (type == LAKE)
	{
		float speed = 0.50f;
		float frequencyX = 1.25f;
		float frequencyY = 0.40f;
		
		float scaleSpeedX = 0.25f;
		float scaleSpeedY = 0.33f;
		
		float factorMin = 0.80;
		float factorScale = 0.20f;
		float factor = factorMin + factorScale * cos(fragment_position.x*frequencyX*cos(gameTime * scaleSpeedX) + gameTime*speed) * sin(fragment_position.y*frequencyY*sin(gameTime*scaleSpeedY) + gameTime*speed);
		color = vec4(vec3(color) * scaleFactor * vec3(typeColor), 1.0);
		color.r *= factor;
		color.g *= 1.0f;
		color.b *= factor;
	}
    else if (type == RIVER)
    {
        // Brighter and faster than LAKE
        float speed = 10.0f;
		float frequencyX = 2.25f;
		float frequencyY = 1.40f;
		
		float scaleSpeedX = 0.45f;
		float scaleSpeedY = 0.43f;
		
		float factorMin = 0.70;
		float factorScale = 0.30f;
		float factor = factorMin + factorScale * cos(fragment_position.x*frequencyX*cos(gameTime * scaleSpeedX) + gameTime*speed) * sin(fragment_position.y*frequencyY*sin(gameTime*scaleSpeedY) + gameTime*speed);
		color = vec4(vec3(color) * scaleFactor * vec3(typeColor), 1.0);
		color.r *= factor;
		color.g *= 1.0f;
		color.b *= factor;
    }
	else
	{
		color = vec4(vec3(color) * scaleFactor * vec3(typeColor), 1.0);
	}
    
    /*
    // Apply lighting calculations.
    color = vec4(0.5f, 0.5f, 0.5f, 1.0f); 
    color *= scaleFactor;
    color.w = 1.0;
    
    */
    /*
    if (true) // fract(vertexId) < 0.2)
    {
        color = vec4(vec3((( - min) / (max - min)) * 0.6 + 0.2), 1.0);
    }
    else
    {
        color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }*/
}

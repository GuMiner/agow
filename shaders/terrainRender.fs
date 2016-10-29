#version 400 core

// Use the terrain texture for fragment shading. TODO this eventually will be a lot more complicated / interesting and use the other color components.
uniform sampler2D terrainTexture;
uniform sampler2D terrainType;

uniform float gameTime;

smooth in vec2 tc_fs;
smooth in vec3 fragment_position;

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

//-------------------------------------------------------------------------------------------------
//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
//-------------------------------------------------------------------------------------------------

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
		float speed = 0.080f;
		// float frequencyX = 1.25f;
		// float frequencyY = 0.40f;
		// 
		// float scaleSpeedX = 0.25f;
		// float scaleSpeedY = 0.33f;
		// 
		float factorMin = 0.80;
		float factorScale = 0.80f;
        float noiseScale = 0.20f;
        
        float colorSpeed = 1.0f;
        
		// float factor = factorMin + factorScale * cos(fragment_position.x*frequencyX*cos(gameTime * scaleSpeedX) + gameTime*speed) * sin(fragment_position.y*frequencyY*sin(gameTime*scaleSpeedY) + gameTime*speed);
        float factor = factorMin + factorScale * snoise(fragment_position.xy * noiseScale + vec2(gameTime * speed)) * (0.3f + abs(sin(gameTime * colorSpeed)));
		color = vec4(vec3(color) * vec3(typeColor), 0.96f);
		color.r *= factor;
		color.g *= 1.0f;
		color.b *= factor;
	}
    else if (type == RIVER)
    {
        // Brighter and faster than LAKEdd
        float speed = 0.02f;
		float frequencyX = 0.225f;
		float frequencyY = 0.140f;
		
		float scaleSpeedX = 0.45f;
		float scaleSpeedY = 0.43f;
		
		float factorMin = 0.70;
		float factorScale = 0.30f;
		float factor = factorMin + factorScale * cos(fragment_position.x*frequencyX*cos(gameTime * scaleSpeedX) + gameTime*speed) * sin(fragment_position.y*frequencyY*sin(gameTime*scaleSpeedY) + gameTime*speed);
		
        int gameTimeRotation = int((gameTime - 0.5f) * 50 + fragment_position.x * 50 + 
            + snoise(fragment_position.xy) * 20) % 180;
        float waveletFactor = gameTimeRotation < 90 ? sin(gameTime) : float(180 - gameTimeRotation) / (90.0f * 1.0f);
        color = vec4(vec3(color) * vec3(typeColor), 1.0);
		color.r *= factor + waveletFactor * 0.15f;
		color.g *= 0.75f + waveletFactor * 0.25f;
		color.b *= factor + waveletFactor * 1.1f;
        color.a = 0.95f;
    }
	else
	{
		color = vec4(vec3(color) * vec3(typeColor), 1.0);
	}
}

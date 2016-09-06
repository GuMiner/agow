#version 400 core

uniform vec4 flavorColor;

in vec3 uvwPos;

out vec4 color;

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

float normalNoise(vec2 v)
{
    return (snoise(v) / 0.5f) + 0.5f;
}

// Creates pseudorandom stars.
void main(void)
{
    
    // Map our cube to a sphere.
    vec3 spherePos = normalize(uvwPos);
    
    vec3 realColor = spherePos / 0.5f + vec3(0.5f);
    color = vec4(realColor, 1.0f);
    float phi = atan(spherePos.y, spherePos.x) + 3.14159f; // 0 - 2pi
    float theta = acos(spherePos.z); // 0 to pi.
    
    // These factors control the star size and shape.
	ivec2 uvwId = ivec2(phi * 140, theta * 220);
	bool isStar = 
        uvwId.x % 3 == 0 && 
        uvwId.y % 5 == 0 && 
        normalNoise(vec2(uvwId.x, uvwId.y) + vec2(uvwId.x * uvwId.y, -uvwId.x * uvwId.y)) > 1.55f;
	
	// Generate a 'random' star color.
	vec3 starColor = 
		vec3(0.75f + 0.25f * normalNoise(vec2(uvwId.x + 7.0f, uvwId.y)),
			 0.75f + 0.25f * normalNoise(vec2(uvwId.x + 5.0f, uvwId.y)),
			 0.75f + 0.25f * normalNoise(vec2(uvwId.x + 3.0f, uvwId.y)));
			 
	vec3 backgroundColor = vec3(0.078f, 0.047f, 0.188f);
	
	if (isStar)
	{
		color = vec4(starColor, 1.0f);
		
		// Apply the flavor color to all the stars found.
		color = color * vec4(flavorColor.xyz, 1.0f) * flavorColor.w;
	}
    else
	{
		color = vec4(backgroundColor, 1.0f);
	}
}
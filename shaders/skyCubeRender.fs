#version 400 core

uniform vec4 flavorColor;
in vec3 uvwPos;

out vec4 color;

void main(void)
{
	ivec3 uvwId = ivec3(uvwPos.x * 100, uvwPos.y * 100, uvwPos.z * 100);
	bool isStar = sin(dot(uvwId.xy, vec2(7.0f, 5.0f))*dot(uvwId.yz, vec2(3.0f, 11.0f)) * uvwId.x) > 0.99999f;
	
	// Generate a 'random' star color.
	vec3 starColor = 
		vec3(0.75f + 0.25f * sin(dot(uvwId.xy, vec2(3.0f, 5.0f)) * 2000.0f),
			 0.75f + 0.25f * sin(dot(uvwId.xy, vec2(9.0f, 5.0f)) * 2000.0f),
			 0.75f + 0.25f * cos(dot(uvwId.xy, vec2(3.0f, 7.0f)) * 2000.0f));
			 
	vec3 backgroundColor = vec3(0.078f, 0.047f, 0.188f);
	
	if (isStar)
	{
		color = vec4(starColor, 1.0f);
	}
    else
	{
		color = vec4(backgroundColor, 1.0f);
	}
	
	// Apply the flavor color to all the stars found.
	color = color * vec4(flavorColor.xyz, 1.0f) * flavorColor.w;
}
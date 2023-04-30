#version 450

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;
in vec4 FragPosLightSpace;

// first texture!
uniform sampler2D diffuseTex;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	
	float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;  
	return shadow;
}

void main()
{
	// Calculate ShadowCalculation
	float shadow = ShadowCalculation(FragPosLightSpace);
	vec4 texColour = texture(diffuseTex, _TexCoords);
	FragColour = vec4(texColour.xyz * (1.0 - shadow), 1);
}
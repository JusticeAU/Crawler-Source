#version 450

out vec4 FragColour;

in vec3 _Position;
in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;
in vec4 _FragPosLightSpace;

// Lights 
uniform vec3 ambientLightColour;
uniform vec3 sunLightDirection;
uniform vec3 sunLightColour;

// Point Lights
const int MAX_LIGHTS = 4;
uniform int numLights;
uniform vec3 PointLightColours[MAX_LIGHTS];
uniform vec3 PointLightPositions[MAX_LIGHTS];

// first texture!
uniform sampler2D diffuseTex;
uniform sampler2D shadowMap;

uniform float shadowBias;

// Functions
vec3 GetDiffuse(vec3 direction, vec3 colour, vec3 normal)
{
	return colour * max(0.0, dot( normal, -direction ) );
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	// if the shadow is outside of the orthographic far clip
	if(projCoords.z > 1.0) return 0.0;
        
	
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	
	// if we get the direction in we can do this a bit more smartly with a dot product
	//float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
	float shadow = currentDepth - shadowBias > closestDepth  ? 1.0 : 0.0;
	
	// Simple PCF
	shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	
	return shadow;
}

void main()
{
	// Normalize all direction vectors
	vec3 N = normalize(_Normal);
	vec3 L = normalize(sunLightDirection);

	// calculate diffuse lighting from sunlight 
	vec3 diffuseTotal = GetDiffuse(L, sunLightColour, N);
	
	// Calculate diffuse  for all point lights
	for(int i = 0; i < numLights; i++)
	{
		vec3 direction = _Position.xyz - PointLightPositions[i]; // get direction from pixel to light (not normalized)
		float distance = length(direction);
		direction = direction / distance; // normalize

		// Attenuate the light intensity with inverese square law
		vec3 colour = PointLightColours[i] / (distance * distance);

		diffuseTotal += GetDiffuse(direction, colour, N);
	}
	
	float shadow = ShadowCalculation(_FragPosLightSpace);
	

	vec3 texColour = texture(diffuseTex, _TexCoords).rgb;
	vec3 lighting = (1.0 - shadow) * diffuseTotal * texColour;  
	FragColour = vec4(lighting, 1);	
}
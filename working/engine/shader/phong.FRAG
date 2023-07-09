#version 450

out vec4 FragColour;

in vec3 _Position;
in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;
in vec4 _FragPosLightSpace;

in vec3 _Tangent;
in vec3 _BiTangent;

uniform vec3 cameraPosition;

// Lights 
uniform vec3 ambientLightColour;
uniform vec3 sunLightDirection;
uniform vec3 sunLightColour;

// Point Lights
const int MAX_LIGHTS = 4;
uniform int numLights;
uniform vec3 PointLightColours[MAX_LIGHTS];
uniform vec3 PointLightPositions[MAX_LIGHTS];

// Material Properties
uniform vec3 Ka; // ambient material colour
uniform vec3 Kd; // diffuse material colour
uniform vec3 Ks; // specular material colour
uniform float specularPower; // material specular power

// Textures
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;

uniform sampler2D shadowMap;
uniform float shadowBias;

// Functions
vec3 GetDiffuse(vec3 direction, vec3 colour, vec3 normal)
{
	return colour * max( 0, dot( normal, -direction ) );
}

vec3 GetSpecular(vec3 direction, vec3 colour, vec3 normal, vec3 view)
{
	vec3 R = reflect( direction, normal );
	float specularTerm = pow( max( 0, dot( R, view ) ), specularPower );
	return specularTerm * colour;
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
	vec3 T = normalize(_Tangent);
	vec3 B = normalize(_BiTangent);
	vec3 L = normalize(sunLightDirection);

	// Construct TBN matrix for normal mapping
	mat3 TBN = mat3(T,B,N);

	// Calculate new normal from normal map
	vec3 texNormal = texture( normalTex, _TexCoords ).rgb;
	N = TBN * (texNormal * 2 - 1);

	// View and Reflection vector for specular
	vec3 V = normalize(cameraPosition - _Position);
	vec3 R = reflect(L, N);

	// calculate diffuse lighting from sunlight 
	vec3 diffuseTotal = GetDiffuse(L, sunLightColour, N);
	
	// Calculate specular total start
	vec3 specularTotal = GetSpecular(L, sunLightColour, N, V);

	// Calculate diffuse and specular for all point lights
	for(int i = 0; i < numLights; i++)
	{
		vec3 direction = _Position.xyz - PointLightPositions[i]; // get direction from pixel to light (not normalized)
		float distance = length(direction);
		direction = direction / distance; // normalize

		// Attenuate the light intensity with inverese square law
		vec3 colour = PointLightColours[i] / (distance * distance);

		diffuseTotal += GetDiffuse(direction, colour, N);
		specularTotal += GetSpecular(direction, colour, N, V);
	}

	// Diffuse
	vec3 texDiffuse = texture(diffuseTex, _TexCoords).rgb;
	vec3 diffuse = Kd * texDiffuse * diffuseTotal;

	// Ambient
	vec3 ambient = ambientLightColour * Ka * texDiffuse;

	// Specular
	vec3 texSpecular = texture(specularTex, _TexCoords).rgb;
	vec3 specular = Ks * texSpecular * specularTotal;
	
	float shadow = ShadowCalculation(_FragPosLightSpace);

	// Output
	FragColour = vec4((ambient + diffuse + specular) * (1.0 - shadow), 1) ;
}
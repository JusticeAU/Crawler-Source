#version 450

out vec4 FragColour;

in vec3 _Position;
in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;

uniform vec3 cameraPosition;

// Lights 
uniform vec3 ambientLightColour;
uniform vec3 sunLightDirection;
uniform vec3 sunLightColour;

// Material Properties
uniform vec3 Ka; // ambient material colour
uniform vec3 Kd; // diffuse material colour
uniform vec3 Ks; // specular material colour
uniform float specularPower; // material specular power

// first texture!
uniform sampler2D diffuseTex;

// Phong Lighting = Ambient + Diffuse(All Lights) + Specular (All Lights)

void main()
{
	// Normalize all direction vectors
	vec3 N = normalize(_Normal);
	vec3 L = normalize(sunLightDirection);

	// Lambert Term
	float lambertTerm = clamp(dot(N, -L), 0.0, 1.0);

	// View and Reflection vector for specular
	vec3 V = normalize(cameraPosition - _Position);
	vec3 R = reflect(L, N);
	float specularTerm = pow(max(0, dot(R, V ) ), specularPower);

	// diffuse
	vec3 texColour = texture(diffuseTex, _TexCoords).rgb;
	vec3 diffuse = sunLightColour * Kd * lambertTerm * texColour;

	// Ambient
	vec3 ambient = ambientLightColour * Ka * texColour;

	// Specular
	vec3 specular = sunLightColour * Ks * specularTerm;

	// Output
	FragColour = vec4(ambient + diffuse + specular, 1);
}
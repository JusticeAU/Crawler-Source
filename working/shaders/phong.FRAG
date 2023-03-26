#version 450

out vec4 FragColour;

in vec3 _Position;
in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;

in vec3 _Tangent;
in vec3 _BiTangent;

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

// Textures
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;

// Phong Lighting = Ambient + Diffuse(All Lights) + Specular (All Lights)

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

	// Lambert Term
	float lambertTerm = clamp(dot(N, -L), 0.0, 1.0);

	// View and Reflection vector for specular
	vec3 V = normalize(cameraPosition - _Position);
	vec3 R = reflect(L, N);
	float specularTerm = pow(max(0, dot(R, V ) ), specularPower);

	// diffuse
	vec3 texDiffuse = texture(diffuseTex, _TexCoords).rgb;
	vec3 diffuse = sunLightColour * Kd * lambertTerm * texDiffuse;

	// Ambient
	vec3 ambient = ambientLightColour * Ka * texDiffuse;

	// Specular
	vec3 texSpecular = texture(specularTex, _TexCoords).rgb;
	vec3 specular = sunLightColour * Ks * specularTerm * texSpecular;

	// Output
	FragColour = vec4(ambient + diffuse + specular, 1);
	//FragColour = vec4(N, 1);
}
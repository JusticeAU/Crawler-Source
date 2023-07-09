#version 450

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;

// Directional light
uniform vec3 sunLightDirection;

// first texture!
uniform sampler2D diffuseTex;

void main()
{
	float lightValue = clamp(dot(-sunLightDirection, _Normal), 0.0, 1.0);
	lightValue += 0.3f;
	vec3 texColour = texture(diffuseTex, _TexCoords).rgb;
	FragColour = vec4(texColour * lightValue, 1);
}
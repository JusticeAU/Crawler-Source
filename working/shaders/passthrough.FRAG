#version 450

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;

// Directional light
uniform vec3 lightDirection;

void main()
{
	float lightValue = clamp(dot(-lightDirection, _Normal), 0.0, 1.0);
	lightValue += 0.1f;
	FragColour = vec4(_Colour, 1);
}
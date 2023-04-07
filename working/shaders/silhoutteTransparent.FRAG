#version 450

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;

uniform vec3 Ka;


void main()
{
	FragColour = vec4(Ka, 0.2); // simply make the alpha low - requires blending enabled.
}
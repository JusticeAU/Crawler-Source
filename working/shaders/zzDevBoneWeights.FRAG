#version 450

const int NUM_BONES = 4;

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;

// first texture!
uniform sampler2D diffuseTex;

void main()
{
	FragColour = vec4(_Colour, 1);
}
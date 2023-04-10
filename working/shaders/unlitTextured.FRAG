#version 450

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;
in vec2 _TexCoords;

// first texture!
uniform sampler2D diffuseTex;

void main()
{
	vec4 texColour = texture(diffuseTex, _TexCoords);
	FragColour = vec4(texColour.xyz, 1);
}
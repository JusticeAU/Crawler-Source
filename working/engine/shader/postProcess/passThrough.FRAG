#version 450

in vec2 _TexCoords;
out vec4 FragColour;

uniform sampler2D frame;

void main()
{
	FragColour = texture(frame, _TexCoords);
}
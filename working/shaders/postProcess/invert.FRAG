#version 450

in vec2 _TexCoords;
out vec4 FragColour;

uniform sampler2D frame;

void main()
{
	vec4 colour = texture(frame, _TexCoords);
	FragColour = vec4(1.0 - colour.rgb, colour.a);
}
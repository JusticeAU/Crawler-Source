#version 450

in vec2 _TexCoords;
out vec4 FragColour;

uniform sampler2D frame;

void main()
{
	vec4 colour = texture(frame, _TexCoords);
	float average = (colour.r + colour.g + colour.b) / 3.0;
	FragColour = vec4(average, average, average, colour.a);
}
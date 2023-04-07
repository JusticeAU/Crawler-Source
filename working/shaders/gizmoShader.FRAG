#version 450

out vec4 FragColour;

in vec3 _Colour;
in vec3 _Normal;

uniform vec3 gizmoColour;

void main()
{
	FragColour = vec4(gizmoColour, 1);
}
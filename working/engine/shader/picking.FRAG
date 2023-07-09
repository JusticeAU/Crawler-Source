#version 450

uniform uint objectID;

out vec4 FragColour;

void main()
{
	FragColour = vec4(float(objectID), 0, 0, 1);
}
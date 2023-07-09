#version 450

layout (location = 0) in vec3 Position;

uniform mat4 pvmMatrix;


void main()
{
	gl_Position = pvmMatrix * vec4(Position ,1);
}
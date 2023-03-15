#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Colour;


uniform float something;
uniform mat4 transformMatrix;

out vec3 _Colour;

void main()
{
	gl_Position = transformMatrix * vec4(Position ,1);
	_Colour = Colour;
}
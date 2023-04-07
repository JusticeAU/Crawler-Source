#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Colour;
layout (location = 2) in vec3 Normal;

uniform mat4 pvmMatrix;
uniform mat4 mMatrix;

out vec3 _Colour;
out vec3 _Normal;

void main()
{
	gl_Position = pvmMatrix * vec4(Position ,1);
	_Colour = Colour;
	_Normal = (mMatrix * vec4(Normal,0)).xyz;
}
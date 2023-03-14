#version 450

layout (location = 0) in vec2 Position;

uniform float something;

void main()
{
	gl_Position = vec4(Position * 0.5f + something,0,1);
}
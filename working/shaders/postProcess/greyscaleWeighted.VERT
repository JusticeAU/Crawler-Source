#version 450
layout (location = 0) in vec3 Position;
layout (location = 3) in vec2 TexCoords;

out vec2 _TexCoords;

void main()
{
	_TexCoords = TexCoords;
	gl_Position = vec4(Position ,1);
}
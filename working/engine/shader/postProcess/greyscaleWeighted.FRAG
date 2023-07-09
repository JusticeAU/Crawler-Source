#version 450

in vec2 _TexCoords;
out vec4 FragColour;

uniform sampler2D frame;

void main()
{
	vec4 colour = texture(frame, _TexCoords);
	//weighted average - https://learnopengl.com/Advanced-OpenGL/Framebuffers
	float average = (0.2126 * colour.r + 0.7152 * colour.g + 0.0722 * colour.b) / 3.0;
	FragColour = vec4(average, average, average, colour.a);
}
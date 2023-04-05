#version 450

const int NUM_BONES = 4;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Colour;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec2 texCoords;
layout (location = 5) in ivec4 boneIDs;
layout (location = 6) in vec4 boneWeights;

uniform mat4 transformMatrix;
uniform mat4 mMatrix;

out vec3 _Colour;
out vec3 _Normal;
out vec2 _TexCoords;

uniform int selectedBone;

void main()
{
	gl_Position = transformMatrix * vec4(Position ,1);
	_Colour = vec3(0);
	for(int i = 0; i < 4; i++)
	{
		if(boneIDs[i] == selectedBone)
		{
			_Colour = vec3(boneWeights[i]);
		}
	}

	_Normal = (mMatrix * vec4(Normal,0)).xyz;
	_TexCoords = texCoords;
}
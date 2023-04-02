#version 450

const int MAX_BONES = 50;

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
uniform mat4x4[MAX_BONES] boneTransforms;

void main()
{
	mat4 boneTransform = boneTransforms[boneIDs[0]] * boneWeights[0];
	if(boneTransforms[boneIDs[1]] != -1) boneTransform += boneTransforms[boneIDs[1]] * boneWeights[1];
    if(boneTransforms[boneIDs[2]] != -1) boneTransform += boneTransforms[boneIDs[2]] * boneWeights[2];
    if(boneTransforms[boneIDs[3]] != -1) boneTransform += boneTransforms[boneIDs[3]] * boneWeights[3];

	vec4 posTransformed = boneTransform * vec4(Position, 1);

	gl_Position = transformMatrix * posTransformed;

	vec4 normalTransformed = boneTransform * vec4(Normal, 1);
	_Normal = (mMatrix * normalTransformed).xyz;
	_TexCoords = texCoords;
}
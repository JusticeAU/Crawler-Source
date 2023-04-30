#version 450

const int MAX_BONES = 100;

layout (location = 0) in vec3 Position;
layout (location = 5) in ivec4 boneIDs;
layout (location = 6) in vec4 boneWeights;
layout (std140) uniform boneTransformBuffer
{
    mat4x4[MAX_BONES] bones;
};

uniform mat4 pvmMatrix;


void main()
{
	mat4 boneTransform = bones[boneIDs[0]] * boneWeights[0];
		boneTransform += bones[boneIDs[1]] * boneWeights[1];
		boneTransform += bones[boneIDs[2]] * boneWeights[2];
		boneTransform += bones[boneIDs[3]] * boneWeights[3];
		
	vec4 posTransformed = boneTransform * vec4(Position, 1);

	gl_Position = pvmMatrix * posTransformed;
}
#version 450

const int MAX_BONES = 100;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Colour;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec2 texCoords;
layout (location = 4) in vec4 Tangent;
layout (location = 5) in ivec4 boneIDs;
layout (location = 6) in vec4 boneWeights;
layout (std140) uniform boneTransformBuffer
{
    mat4x4[MAX_BONES] bones;
};

uniform mat4 pvmMatrix;
uniform mat4 mMatrix;

out vec3 _Position;
out vec3 _Colour;
out vec3 _Normal;
out vec2 _TexCoords;

out vec3 _Tangent;
out vec3 _BiTangent;

uniform int selectedBone;

void main()
{
	mat4 boneTransform = bones[boneIDs[0]] * boneWeights[0];
	boneTransform += bones[boneIDs[1]] * boneWeights[1];
    boneTransform += bones[boneIDs[2]] * boneWeights[2];
    boneTransform += bones[boneIDs[3]] * boneWeights[3];

	vec4 posTransformed = boneTransform * vec4(Position, 1);
	vec4 normalTransformed = boneTransform * vec4(Normal, 0);
	vec4 tangentTransformed = boneTransform * vec4(Tangent.xyz, 0);

	_Position = (mMatrix * posTransformed).xyz;
	_Normal = (mMatrix * normalTransformed).xyz;
	_TexCoords = texCoords;
	_Tangent = (mMatrix * tangentTransformed).xyz;
	_BiTangent = cross(_Normal, _Tangent) * Tangent.w;
	
	_Colour = Colour;
	gl_Position = pvmMatrix * posTransformed;
}
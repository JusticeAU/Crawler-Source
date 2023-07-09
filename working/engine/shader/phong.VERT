#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Colour;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec2 texCoords;
layout (location = 4) in vec4 Tangent;

uniform mat4 pvmMatrix;
uniform mat4 mMatrix;
uniform mat4 lightSpaceMatrix; // for single shadow map translation

out vec3 _Position;
out vec3 _Colour;
out vec3 _Normal;
out vec2 _TexCoords;
out vec4 _FragPosLightSpace;

out vec3 _Tangent;
out vec3 _BiTangent;

void main()
{
	vec3 FragPos = vec3(mMatrix * vec4(Position, 1.0));

	_Position = (mMatrix * vec4(Position, 1)).xyz;
	_Colour = Colour;
	_Normal = (mMatrix * vec4(Normal,0)).xyz;

	_TexCoords = texCoords;

	_Tangent = (mMatrix * vec4(Tangent.xyz,0)).xyz;
	_BiTangent = cross(_Normal, _Tangent) * Tangent.w;
	
	gl_Position = pvmMatrix * vec4(Position ,1);
	
	_FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}
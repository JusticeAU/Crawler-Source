#pragma once
#include "Mesh.h"
#include <map>
#include <vector>

class Mesh;
class Animation;
class ShaderProgram;

class Object;

using std::vector;
using std::map;

// A model is a container for meshes, bone structures and animation data. They are created by the Model Manager and data populated during import.
class Model
{
public:
	

	struct Animation // an animation contains some meta data and a collection of channels (bones) which contain a collection of keys (position, rotation, scale).
	{
		struct AnimationKey // a key is a keyframe of information about a particular bone.
		{
			vec3 position;
			quat rotation;
			vec3 scale;
		};

		struct AnimationChannel // a channel is a bone
		{
			string name;
			AnimationKey keys[500];
		};

		string name;
		float duration;
		float ticksPerSecond;
		int numChannels;
		map<string, AnimationChannel> channels;
	};

	struct BoneStructure
	{
		int numBones = 0;
		map<string, int> boneMapping;	// boneName and index pair. The index is useed to address in to boneOffset array and assign transformations in to the buffer for the vertex shader.
		vector<glm::mat4> boneOffsets;	// contains the offset for the bone.
	};

	vector<Mesh*> meshes;
	vector<Animation*> animations;
	
	Object* childNodes;
	BoneStructure* boneStructure;

	void Draw();
};

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

using glm::mat4;

// A model is a container for meshes, bone structures and animation data. They are created by the Model Manager and data populated during import.
class Model
{
public:
	struct Animation // an animation contains some meta data and a collection of channels (bones) which contain a collection of keys (position, rotation, scale).
	{
		struct AnimationKey // a key is a keyframe of information about a particular bone.
		{
			float time;
			vec3 position;
			quat rotation;
			vec3 scale;
		};

		struct AnimationChannel // a channel describes animation keys for a particular bone.
		{
			string name;
			vector<AnimationKey> keys; // keys are stored in order, but are displaced by their duration. key 1-2 might have a bigger gap than key 2-3. GetTransformation handles this.

			mat4 GetTransformation(float t)
			{
				AnimationKey* to = nullptr; // in is the frame we are lerping in to.
				AnimationKey* from = nullptr;

				for (int i = 0; i < keys.size(); i++)
				{
					if (keys[i].time >= t) // found key we're moving to. It is the AnimatorComponents job not to overflow this operation. (don't go beyond animation duration)
					{
						to = &keys[i];
						from = i > 0 ? &keys[i - 1] : &keys[keys.size() - 1]; // If we landed on the first(0th) key, then we should be blending from the last key - which is likely the same data but shouldnt be assumed.
						break;
					}
				}

				if (to != nullptr && from != nullptr)
				{
					// calculate t2 (position between keys) with an inverse lerp
					float t2 = (t - from->time) / (to->time - from->time);

					// mix transformations based on t2 and return combination.
					mat4 scale = glm::scale(mat4(1), glm::mix(from->scale, to->scale, t2));					// generate mixed scale matrix		
					mat4 rotate = glm::mat4_cast(glm::slerp(from->rotation, to->rotation, t2));					// generate mixed rotation matrix
					mat4 translate = glm::translate(mat4(1), glm::mix(from->position, to->position, t2));	// generate mixed translation matrix
					return translate * rotate * scale;															// combine
				}
				else
				{
					return mat4(1);
				}
			}
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


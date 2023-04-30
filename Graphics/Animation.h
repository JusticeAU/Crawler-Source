#pragma once
#include "Graphics.h"
#include <map>
#include <string>
#include <vector>

class Mesh;
class Animation;
class ShaderProgram;

class Object;

using std::string;
using std::vector;
using std::map;

using glm::vec3;
using glm::quat;
using glm::mat4;


class Animation // an animation contains some meta data and a collection of channels (bones) which contain a collection of keys (position, rotation, scale).
{
public:
	struct AnimationKey // a key is a keyframe of information about a particular bone.
	{
		float time = 0.0f;
		vec3 position = vec3(0);
		quat rotation = quat();
		vec3 scale = vec3(0);;
	};

	struct AnimationChannel // a channel describes animation keys for a particular bone.
	{
		string name = "";
		vector<AnimationKey> keys; // keys are stored in order, but are displaced by their duration. key 1-2 might have a bigger gap than key 2-3. GetTransformation handles this.

		AnimationKey GetKeyAtTime(float t);
	};

	string name = "";
	float duration = 0.0f;
	float ticksPerSecond = 0.0f;
	int numChannels = 0;
	map<string, AnimationChannel> channels;
};

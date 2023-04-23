#include "Animation.h"

mat4 Animation::AnimationChannel::GetTransformation(float t)
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

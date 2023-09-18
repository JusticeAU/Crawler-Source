#include "ComponentAnimator.h"
#include "ComponentModel.h"
#include "Model.h"
#include "Animation.h"
#include "Object.h"
#include "UniformBuffer.h"
#include "ModelManager.h"

#include "LogUtils.h"

using std::to_string;

ComponentAnimator::ComponentAnimator(Object* parent) : Component("Animator", Component_Animator, parent)
{
	current = new AnimationState();

	boneTransforms = new mat4[MAX_BONES]();
	boneTransfomBuffer = new UniformBuffer(sizeof(mat4) * MAX_BONES);
}

ComponentAnimator::ComponentAnimator(Object* parent, ordered_json j) : ComponentAnimator(parent)
{
	
	if (j.contains("name"))
	{
		string animationName;
		j.at("name").get_to(animationName);
		StartAnimation(animationName, true);
	}
	if (j.contains("speed"))
		j.at("speed").get_to(current->animationSpeedScale);
	
}

ComponentAnimator::~ComponentAnimator()
{
	delete boneTransforms;
	delete boneTransfomBuffer;
	AnnounceChange();
}

void ComponentAnimator::Update(float delta)
{
	if (!model) // Can't do anything without a model.
		return;

	if (current->animation == nullptr && model->animations.size() > 0) // pick first animation if we dont have one.
		current->animation = model->animations[0];
	
	if (isPlaying)
	{
		if (current->animation)
			current->Update(delta);

		if (next) // if we have an animation to transition to
		{
			next->Update(delta);
			transitionProgress += delta;

			if (transitionProgress >= transitionTime)
			{
				current = next;
				next = nullptr; // TO DO animation manager? ahahahaha more managers.
				transitionProgress = 0.0f;
			}

			transitionWeight = transitionProgress / transitionTime;
		}
	}

	// Update animation state, if we have an animation
	if (current->animation != nullptr) // We have an animation
	{
		// Update the array of transform matricies - this is sent in to the shader when Draw is called.
		// Actually dont need to send this argument in given that its a member function - will refactor this in to an animator component at some point.
		UpdateBoneMatrixBuffer();
	}
}

void ComponentAnimator::DrawGUI()
{
	// Animation information.
	if (model != nullptr && model->boneStructure != nullptr)
	{
		if (model->animations.size() > 0)
		{
			ImGui::Text("Playback Settings");
			string AnimSpeedStr = "Animation Speed##" + to_string(componentParent->id);
			ImGui::DragFloat(AnimSpeedStr.c_str(), &current->animationSpeedScale, 0.1f, -2, 2);
			ImGui::Text("");
			ImGui::Text("Blend Settings");
			ImGui::Checkbox("Next Anim Should Loop?", &transitionsShouldLoop);
			ImGui::InputFloat("Next Anim Blend Time", &transitionBlendTime);
			string animationNameStr = "Animation##" + to_string(componentParent->id);
			if (ImGui::BeginCombo(animationNameStr.c_str(), current->animation->name.c_str()))
			{
				for (auto a : *ModelManager::Animations())
				{
					const bool is_selected = (a.second == current->animation);
					if (ImGui::Selectable(a.first.c_str(), is_selected))
						BlendToAnimation(a.first, transitionBlendTime, 0.0f, transitionsShouldLoop);

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

			}

			ImGui::Text("");
			ImGui::Text("Current Animation");
			string AnimLoopStr = "Loop##" + to_string(componentParent->id);
			ImGui::Checkbox(AnimLoopStr.c_str(), &current->looping);
			ImGui::SameLine();
			string AnimPlayStr = isPlaying ? "Pause##" + to_string(componentParent->id) : "Play##" + to_string(componentParent->id);
			if (ImGui::Button(AnimPlayStr.c_str())) isPlaying = !isPlaying;

			string animTimeStr = "Animation Time##" + to_string(componentParent->id);
			ImGui::SliderFloat(animTimeStr.c_str(), &current->position, 0, current->animation->duration);

			// blending
			if (next)
			{
				string nextName = "Blending to.." + next->animation->name;
				ImGui::Text(nextName.c_str());
				ImGui::BeginDisabled();
				ImGui::SliderFloat("Blend", &transitionWeight,0, 1);
				ImGui::EndDisabled();
			}
		}
	}
}

void ComponentAnimator::OnParentChange()
{
	Component* component = componentParent->GetComponent(Component_Model);
	if (component)
	{
		model = static_cast<ComponentModel*>(component)->model;
	}
	else
		model = nullptr;
}

Component* ComponentAnimator::Clone(Object* parent)
{
	ComponentAnimator* copy = new ComponentAnimator(parent);
	copy->model = model;
	return copy;
}


// This proceses the Animation data to build a new boneTransform array to send to the GPU.
void ComponentAnimator::UpdateBoneMatrixBuffer()
{
	if (model == nullptr)
		return;
	if (model->boneStructure == nullptr)
		return;

	ProcessNode(model->childNodes, mat4(1));
}

// recursively processes each node/bone.
void ComponentAnimator::ProcessNode(Object* node, mat4 accumulated)
{
	// look up if node has a matching bone mapping.
	string nodeName = node->objectName;
	mat4 nodeTransformation = node->localTransform; // Assume it doesnt at first and just use its local transform. (which should be its base offset data)
	Animation::AnimationKey key;
	auto bufferIndex = model->boneStructure->boneMapping.find(nodeName); // first search the bonemap for a valid index. We can't do anything with the data if it's not mapped in to the buffer.
	if (bufferIndex != model->boneStructure->boneMapping.end()) // if it has one, look up its keyframe data.
	{
		// Get key from animation
		Animation::AnimationKey A;
		auto channelA = current->animation->channels.find(nodeName);
		if (channelA != current->animation->channels.end())
		{
			A = channelA->second.GetKeyAtTime(current->position);
			mat4 scale = glm::scale(mat4(1), A.scale);					// generate mixed scale matrix		
			mat4 rotate = glm::mat4_cast(A.rotation);					// generate mixed rotation matrix
			mat4 translate = glm::translate(mat4(1), A.position);	// generate mixed translation matrix
			nodeTransformation = translate * rotate * scale;
		}
		
		Animation::AnimationKey B;
		if (next != nullptr)
		{
			auto channelB = next->animation->channels.find(nodeName);
			if (channelB != next->animation->channels.end())
			{
				B = channelB->second.GetKeyAtTime(next->position);
				Animation::AnimationKey mixed;
				mixed.scale = glm::mix(A.scale, B.scale, transitionWeight);					// generate mixed scale		
				mixed.rotation = glm::slerp(A.rotation, B.rotation, transitionWeight);		// generate mixed rotation
				mixed.position = glm::mix(A.position, B.position, transitionWeight);			// generate mixed translation
			
				mat4 scale = glm::scale(mat4(1), mixed.scale);					// generate mixed scale matrix		
				mat4 rotate = glm::mat4_cast(mixed.rotation);					// generate mixed rotation matrix
				mat4 translate = glm::translate(mat4(1), mixed.position);	// generate mixed translation matrix
				nodeTransformation = translate * rotate * scale;
			}
		}
	}

	mat4 globalTransform = accumulated * nodeTransformation; // Apply matrix to accumulated transform down the tree.

	// if it was an actual bone - apply it to the transform buffer that gets sent to the vertex shader.
	if (bufferIndex != model->boneStructure->boneMapping.end())
		boneTransforms[bufferIndex->second] = globalTransform * model->boneStructure->boneOffsets[bufferIndex->second];

	// Process children.
	for (auto c : node->children)
		ProcessNode(c, globalTransform);

}

void ComponentAnimator::StartAnimation(string name, bool loop)
{
	Animation* animation = ModelManager::GetAnimation(name);
	if (animation == nullptr)
		return;

	AnimationState* newAnimation = new AnimationState();
	newAnimation->animation = animation;
	newAnimation->looping = loop;

	delete current;
	current = newAnimation;
	isPlaying = true;
}

void ComponentAnimator::BlendToAnimation(string name, float time, float offset, bool loop)
{
	if (next != nullptr) // If we're already in the process of blending to an animation, just terminate the blend and set the next animation as current.
		current = next;

	Animation* animation = ModelManager::GetAnimation(name);
	if (animation == nullptr)
	{
		LogUtils::Log("Attemping to play animation that does not exist: " + name);
		return;
	}

	isPlaying = true;
	AnimationState* newAnimation = new AnimationState();
	newAnimation->animation = animation;
	newAnimation->looping = loop;
	next = newAnimation;
	transitionTime = time;
}

void ComponentAnimator::SetPose(string name, float offset)
{
	StartAnimation(name);
	current->position = offset;
	Update(0.0f);
	isPlaying = false;
}

void ComponentAnimator::AnimationState::Update(float delta)
{
	position += delta * animationSpeedScale * animation->ticksPerSecond;
	if (position > animation->duration)
	{
		if (looping)
			position -= animation->duration;
		else
			position = animation->duration;
	}
	else if (position < 0.0f)
	{
		if (looping)
			position += animation->duration;
		else
			position = 0.0f;
	}
}

bool ComponentAnimator::AnimationState::IsFinished()
{
	return position >= animation->duration;
}

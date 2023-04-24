#include "ComponentAnimator.h"
#include "ComponentModel.h"
#include "Model.h"
#include "Animation.h"
#include "Object.h"
#include "UniformBuffer.h"

#include "ModelManager.h"

using std::to_string;

ComponentAnimator::ComponentAnimator(Object* parent, std::istream& istream) : ComponentAnimator(parent)
{
	FileUtils::ReadInt(istream, selectedAnimation);
	FileUtils::ReadString(istream, animationName);
	FileUtils::ReadFloat(istream, animationSpeed);
	current = new AnimationState();
}

ComponentAnimator::~ComponentAnimator()
{
	delete boneTransforms;
	delete boneTransfomBuffer;
	AnnounceChange();
}

void ComponentAnimator::Update(float delta)
{

	if (current)
	{
		if (current->animation == nullptr && model->animations.size() > 0)
		{
			current->animation = model->animations[selectedAnimation];
		}
		if(current->animation != nullptr)
			current->Update(delta);
	}
	
	if (next)
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

	// Update animation state, if we have an animation
	if (model != nullptr && model->animations.size() > 0) // We have animations
	{
		if (boneTransforms == nullptr) // only create a boneTransform matrix array and buffer if we need it.
		{
			boneTransforms = new mat4[MAX_BONES]();
			boneTransfomBuffer = new UniformBuffer(sizeof(mat4) * MAX_BONES);
		}

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
		string boneStr = "Selected Bone##" + to_string(componentParent->id);
		ImGui::DragInt(boneStr.c_str(), &selectedBone, 0.1, 0, model->boneStructure->numBones);

		if (model->animations.size() > 0)
		{
			string animationNameStr = "Animation##" + to_string(componentParent->id);
			if (ImGui::BeginCombo(animationNameStr.c_str(), current->animation->name.c_str()))
			{
				for (auto a : *ModelManager::Animations())
				{
					const bool is_selected = (a.second == current->animation);
					if (ImGui::Selectable(a.first.c_str(), is_selected))
					{
						next = new AnimationState();
						next->animation = a.second;
						next->looping = true;
						//next->position = current->position; // just assume the blended animation is in sync timing wise. like a walk cycle
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

			}

			string AnimSpeedStr = "Animation Speed##" + to_string(componentParent->id);
			ImGui::DragFloat(AnimSpeedStr.c_str(), &current->animationSpeedScale, 0.1f, -2, 2);

			string AnimLoopStr = "Loop##" + to_string(componentParent->id);
			ImGui::Checkbox(AnimLoopStr.c_str(), &current->looping);
			ImGui::SameLine();
			string AnimPlayStr = playAnimation ? "Pause##" + to_string(componentParent->id) : "Play##" + to_string(componentParent->id);
			if (ImGui::Button(AnimPlayStr.c_str())) playAnimation = !playAnimation;

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

void ComponentAnimator::Write(std::ostream& ostream)
{
	FileUtils::WriteInt(ostream, selectedAnimation);
	FileUtils::WriteString(ostream, animationName);
	FileUtils::WriteFloat(ostream, animationSpeed);
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

// This proceses the Animation data to build a new boneTransform array to send to the GPU.
void ComponentAnimator::UpdateBoneMatrixBuffer()
{
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
	

	for (int i = 0; i < model->animations.size(); i++)
	{
		if (model->animations[i]->name == name)
		{
			animationTime = 0.0f;
			playAnimation = true;
			loopAnimation = loop;

			animationName = name;
			selectedAnimation = i;
		}
	}
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
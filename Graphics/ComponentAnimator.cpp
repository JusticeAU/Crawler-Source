#include "ComponentAnimator.h"
#include "ComponentModel.h"
#include "Model.h"
#include "Object.h"
#include "UniformBuffer.h"

using std::to_string;

ComponentAnimator::ComponentAnimator(Object* parent, std::istream& istream) : ComponentAnimator(parent)
{
	FileUtils::ReadInt(istream, selectedAnimation);
	FileUtils::ReadString(istream, animationName);
	FileUtils::ReadFloat(istream, animationSpeed);
}

void ComponentAnimator::Update(float delta)
{
	// Update animation state, if we have an animation
	if (model != nullptr && model->animations.size() > 0) // We have animations
	{
		if (boneTransforms == nullptr) // only create a boneTransform matrix array and buffer if we need it.
		{
			boneTransforms = new mat4[MAX_BONES]();
			boneTransfomBuffer = new UniformBuffer(sizeof(mat4) * MAX_BONES);
		}

		if (selectedAnimation > model->animations.size() - 1) // avoid overflow (could happen if we changed from a model with 3 animations, had the 3rd selected and the new model only had 1 animation.
			selectedAnimation = 0;

		// Process animation state and data.
		if (playAnimation) animationTime += delta * animationSpeed * model->animations[selectedAnimation]->ticksPerSecond;

		// clamp animation time or loop if enabled.
		if (animationTime > model->animations[selectedAnimation]->duration)
		{
			if (loopAnimation)
				animationTime -= model->animations[selectedAnimation]->duration;
			else
				animationTime = model->animations[selectedAnimation]->duration;
		}
		else if (animationTime < 0.0f)
		{
			if (loopAnimation)
				animationTime += model->animations[selectedAnimation]->duration;
			else
				animationTime = 0.0f;
		}

		// Update the array of transform matricies - this is sent in to the shader when Draw is called.
		// Actually dont need to send this argument in given that its a member function - will refactor this in to an animator component at some point.
		UpdateBoneMatrixBuffer(animationTime);
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
			if (ImGui::BeginCombo(animationNameStr.c_str(), model->animations[selectedAnimation]->name.c_str()))
			{
				for (int i = 0; i < model->animations.size(); i++)
				{
					const bool is_selected = (model->animations[i]->name == animationName);
					if (ImGui::Selectable(model->animations[i]->name.c_str(), is_selected))
					{
						selectedAnimation = i;
						animationName = model->animations[i]->name;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

			}

			string AnimSpeedStr = "Animation Speed##" + to_string(componentParent->id);
			ImGui::DragFloat(AnimSpeedStr.c_str(), &animationSpeed, 0.1f, -2, 2);

			string AnimLoopStr = "Loop##" + to_string(componentParent->id);
			ImGui::Checkbox(AnimLoopStr.c_str(), &loopAnimation);
			ImGui::SameLine();
			string AnimPlayStr = playAnimation ? "Pause##" + to_string(componentParent->id) : "Play##" + to_string(componentParent->id);
			if (ImGui::Button(AnimPlayStr.c_str())) playAnimation = !playAnimation;

			string animTimeStr = "Animation Time##" + to_string(componentParent->id);
			ImGui::SliderFloat(animTimeStr.c_str(), &animationTime, 0, model->animations[selectedAnimation]->duration);
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
}

// This proceses the Animation data to build a new boneTransform array to send to the GPU.
void ComponentAnimator::UpdateBoneMatrixBuffer(float frameTime)
{
	ProcessNode(frameTime, selectedAnimation, model->childNodes, mat4(1));
}

// recursively processes each node/bone.
void ComponentAnimator::ProcessNode(float frameTime, int animationIndex, Object* node, mat4 accumulated)
{
	// look up if node has a matching bone/animation node
	int frameIndex = (int)frameTime;
	int nextFrameIndex = frameTime > model->animations[animationIndex]->duration ? 0 : frameIndex + 1;
	float t = frameTime - frameIndex;

	string nodeName = node->objectName;
	mat4 nodeTransformation = node->localTransform; // assume it doesnt at first and just use its local transform.
	auto bufferIndex = model->boneStructure->boneMapping.find(nodeName);
	if (bufferIndex != model->boneStructure->boneMapping.end()) // if it does, look up its keyframe data.
	{
		// Get key from animation
		auto channel = model->animations[animationIndex]->channels.find(nodeName);
		if (channel != model->animations[animationIndex]->channels.end())
		{
			Model::Animation::AnimationKey& key = channel->second.keys[frameIndex];
			Model::Animation::AnimationKey& nextKey = channel->second.keys[nextFrameIndex];

			// Apply transformation.
			mat4 scale = glm::scale(glm::mat4(1), glm::mix(key.scale, nextKey.scale, t));				// generate mixed scale matrix		
			mat4 rotate = glm::mat4_cast(glm::slerp(key.rotation, nextKey.rotation, t));				// generate mixed rotation matrix
			mat4 translate = glm::translate(glm::mat4(1), glm::mix(key.position, nextKey.position, t));	// generate mixed translation matrix
			nodeTransformation = translate * rotate * scale;											// combine
		}
	}

	mat4 globalTransform = accumulated * nodeTransformation;											// Apply matrix to accumulated transform down the tree.

	// if it was an actual bone - apply it the transform buffer that gets sent to the vertex shader.
	if (bufferIndex != model->boneStructure->boneMapping.end())
		boneTransforms[bufferIndex->second] = globalTransform * model->boneStructure->boneOffsets[bufferIndex->second];

	// Process children.
	for (auto c : node->children)
		ProcessNode(frameTime, animationIndex, c, globalTransform);

}
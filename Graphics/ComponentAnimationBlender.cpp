#include "ComponentAnimationBlender.h"
#include "ComponentModel.h"
#include "Model.h"
#include "Animation.h"
#include "Object.h"
#include "UniformBuffer.h"

using std::to_string;

ComponentAnimationBlender::ComponentAnimationBlender(Object* parent, std::istream& istream) : ComponentAnimationBlender(parent)
{
}

ComponentAnimationBlender::~ComponentAnimationBlender()
{
	delete boneTransforms;
	delete boneTransfomBuffer;
}

void ComponentAnimationBlender::Update(float delta)
{
	// Update animation state, if we have an animation
	if (model != nullptr && model->animations.size() > 0) // We have animations
	{
		if (boneTransforms == nullptr) // only create a boneTransform matrix array and buffer if we need it.
		{
			boneTransforms = new mat4[100]();
			boneTransfomBuffer = new UniformBuffer(sizeof(mat4) * 100);
		}


		// Update the array of transform matricies - this is sent in to the shader when Draw is called.
		// Actually dont need to send this argument in given that its a member function - will refactor this in to an animator component at some point.
		UpdateBoneMatrixBuffer();
	}
}

void ComponentAnimationBlender::DrawGUI()
{
	// Animation information.
	if (model != nullptr && model->boneStructure != nullptr)
	{
		if (model->animations.size() > 0)
		{
			ImGui::PushID(componentParent->id);
			if (ImGui::BeginCombo("Animation A", model->animations[animationA]->name.c_str()))
			{
				for (int i = 0; i < model->animations.size(); i++)
				{
					const bool is_selected = (model->animations[i]->name == animationAName);
					if (ImGui::Selectable(model->animations[i]->name.c_str(), is_selected))
					{
						animationA = i;
						animationAName = model->animations[i]->name;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

			}
			ImGui::SliderFloat("Animation A Time", &animationATime, 0, model->animations[animationA]->duration);

			if (ImGui::BeginCombo("Animation B", model->animations[animationB]->name.c_str()))
			{
				for (int i = 0; i < model->animations.size(); i++)
				{
					const bool is_selected = (model->animations[i]->name == animationBName);
					if (ImGui::Selectable(model->animations[i]->name.c_str(), is_selected))
					{
						animationB = i;
						animationBName = model->animations[i]->name;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

			}
			ImGui::SliderFloat("Animation B Time", &animationBTime, 0, model->animations[animationB]->duration);

			ImGui::SliderFloat("Weight AB", &weightAB, 0, 1,"%.2f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::PopID();
		}
	}
}


void ComponentAnimationBlender::OnParentChange()
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
void ComponentAnimationBlender::UpdateBoneMatrixBuffer()
{
	ProcessNode(animationATime, animationA, animationBTime, animationB, weightAB, model->childNodes, mat4(1));
}

// recursively processes each node/bone.
void ComponentAnimationBlender::ProcessNode(float frameTimeA, int animationIndexA, float frameTimeB, int animationIndexB, float weightAB, Object* node, mat4 accumulated)
{
	// look up if node has a matching bone mapping.
	string nodeName = node->objectName;
	mat4 nodeTransformationA = node->localTransform * weightAB; // assume it doesnt at first and just use its local transform. (which should be its base offset data)
	mat4 nodeTransformationB = node->localTransform * (1 - weightAB);

	auto bufferIndex = model->boneStructure->boneMapping.find(nodeName); // first search the bonemap for a valid index. We can't do anything with the data if it's not mapped in to the buffer.
	if (bufferIndex != model->boneStructure->boneMapping.end()) // if it has one, look up its keyframe data.
	{
		// Get key from animation
		auto channelA = model->animations[animationIndexA]->channels.find(nodeName);
		if (channelA != model->animations[animationIndexA]->channels.end())
			nodeTransformationA = channelA->second.GetTransformation(frameTimeA) * weightAB;

		auto channelB = model->animations[animationIndexB]->channels.find(nodeName);
		if (channelB != model->animations[animationIndexB]->channels.end())
			nodeTransformationB = channelB->second.GetTransformation(frameTimeB) * (1 - weightAB);


	}

	mat4 globalTransform = accumulated * (nodeTransformationA + nodeTransformationB); // Apply matrix to accumulated transform down the tree.

	// if it was an actual bone - apply it to the transform buffer that gets sent to the vertex shader.
	if (bufferIndex != model->boneStructure->boneMapping.end())
		boneTransforms[bufferIndex->second] = globalTransform * model->boneStructure->boneOffsets[bufferIndex->second];

	// Process children.
	for (auto c : node->children)
		ProcessNode(frameTimeA, animationIndexA, frameTimeB, animationIndexB, weightAB, c, globalTransform);

}

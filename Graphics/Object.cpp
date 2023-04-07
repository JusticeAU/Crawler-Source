#include "Object.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"

#include "ShaderProgram.h"
#include "UniformBuffer.h"
#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include <string>

using std::to_string;

Object::Object(int objectID, string name)
{
	id = objectID;
	localPosition = { 0,0,0 };
	localRotation = { 0,0,0 };
	localScale = { 1,1,1 };
	transform = mat4(1);
	localTransform = mat4(1);

	objectName = name;

	modelName = "";
	model = nullptr;

	textureName = "";
	texture = nullptr;

	shaderName = "";
	shader = nullptr;

	boneTransforms = nullptr;
}

Object::~Object()
{
	for (auto child : children)
	{
		delete child;
	}

	if (boneTransfomBuffer != nullptr)
	{
		delete boneTransfomBuffer;
		delete[] boneTransforms;
	}

}

void Object::Update(float delta)
{
	if (spin) // just some debug spinning for testing lighting.
	{
		localRotation.y += delta * spinSpeed;
		dirtyTransform = true;

		if (localRotation.y > 180) localRotation.y -= 360;
		else if (localRotation.y < -180) localRotation.y += 360;
	}

	if (dirtyTransform) // Update our transform based on crappy values from ImGui
	{
		localTransform = glm::translate(glm::mat4(1), localPosition)
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.z), glm::vec3{ 0,0,1 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.y), glm::vec3{ 0,1,0 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.x), glm::vec3{ 1,0,0 })
			* glm::scale(glm::mat4(1), localScale);

		// Update world transform based on our parent.
		if (parent)
			transform = parent->transform * localTransform;
		else
			transform = localTransform;

		for (auto c : children)
			c->dirtyTransform = true;

		dirtyTransform = false; // clear dirty flag
	}

	// Update animation state, if we have an animation
	if (model!= nullptr && model->animations.size() > 0) // We have animations
	{
		if (boneTransforms == nullptr)
		{
			boneTransforms = new mat4[MAX_BONES]();
			boneTransfomBuffer = new UniformBuffer(sizeof(mat4) * MAX_BONES); // we need to create buffer to store bonetransforms on the GPU for this object.
		}

		if (selectedAnimation > model->animations.size() - 1)
		{
			selectedAnimation = 0; // avoid overflow
			selectedFrame = 0;
		}

		if(playAnimation) animationTime += delta * animationSpeed * model->animations[selectedAnimation]->ticksPerSecond;
		
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

		selectedFrame = (int)animationTime;
		UpdateBoneMatrixBuffer(animationTime);
	}

	for (auto c : children)
		c->Update(delta);

}

void Object::Draw()
{
	if (model != nullptr && shader != nullptr)
	{
		// Combine the matricies
		glm::mat4 pvm = Camera::s_instance->GetMatrix() * transform;

		shader->Bind();

		// Positions and Rotations
		shader->SetMatrixUniform("transformMatrix", pvm);
		shader->SetMatrixUniform("mMatrix", transform);
		shader->SetVectorUniform("cameraPosition", Camera::s_instance->GetPosition());
	
		// Lighting
		shader->SetVectorUniform("ambientLightColour", Scene::GetAmbientLightColour());
		shader->SetVectorUniform("sunLightDirection", glm::normalize(Scene::GetSunDirection()));
		shader->SetVectorUniform("sunLightColour", Scene::GetSunColour());
		// Point Lights
		int numLights = Scene::GetNumPointLights();
		shader->SetIntUniform("numLights", numLights);
		shader->SetFloat3ArrayUniform("PointLightPositions", numLights, Scene::GetPointLightPositions());
		shader->SetFloat3ArrayUniform("PointLightColours", numLights, Scene::GetPointLightColours());

		// skinned mesh rendering
		shader->SetIntUniform("selectedBone", selectedBone); // dev testing really, used by boneWeights shader
		if (boneTransfomBuffer != nullptr)
		{
			// get the shader uniform block index and set binding point - we'll just hardcode 0 for this.
			shader->SetUniformBlockIndex("boneTransformBuffer", 0);
			// TODO - this could be done in shader initialisation if it detected that that shader had this uniform buffer
			// Now long the UniformBufferObject to the bind point in the GL context.
			boneTransfomBuffer->Bind(0);

			boneTransfomBuffer->SendData(boneTransforms);
		}

		// Texture Uniforms
		if (texture)
		{
			texture->Bind(1);
			shader->SetIntUniform("diffuseTex", 1);
		}

		// Material Uniforms
		if (material)
		{
			shader->SetVectorUniform("Ka", material->Ka);
			shader->SetVectorUniform("Kd", material->Kd);
			shader->SetVectorUniform("Ks", material->Ks);
			shader->SetFloatUniform("specularPower", material->specularPower);

			if (material->mapKd)
			{
				material->mapKd->Bind(1);
				shader->SetIntUniform("diffuseTex", 1);
			}
			if (material->mapKs)
			{
				material->mapKs->Bind(2);
				shader->SetIntUniform("specularTex", 2);
			}
			if (material->mapBump)
			{
				material->mapBump->Bind(3);
				shader->SetIntUniform("normalTex", 3);
			}
		}

		model->Draw();
	}

	for (auto c : children)
		c->Draw();
}

void Object::DrawGUI()
{
	string idStr = to_string(id);
	string objectStr = objectName + "##" + idStr;
	if (ImGui::TreeNode(objectStr.c_str()))
	{

		string childStr = "AddChild##" + to_string(id);
		if (ImGui::Button(childStr.c_str()))
		{
			Object* spawn = Scene::CreateObject(this);
			spawn->Update(0.0f);
		}

		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		string deleteStr = "Delete##" + to_string(id);
		if (ImGui::Button(deleteStr.c_str()))
			markedForDeletion = true;
		string newName = objectName;
		string nameStr = "Name##" + idStr;
		if (ImGui::InputText(nameStr.c_str(), &newName, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			objectName = newName;
		}
		
		if (ImGui::CollapsingHeader("Transform"))
		{
			string positionStr = "Pos##" + to_string(id);
			if (ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]))
				dirtyTransform = true;

			string rotationStr = "Rot##" + to_string(id);
			if(ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180))
				dirtyTransform = true;

			string scaleStr = "Scale##" + to_string(id);
			if(ImGui::DragFloat3(scaleStr.c_str(), &localScale[0]))
				dirtyTransform = true;

			string rotateBoolStr = "Rotate##" + to_string(id);
			ImGui::Checkbox(rotateBoolStr.c_str(), &spin);
			ImGui::SameLine();
			string rotateSpeedStr = "Speed##" + to_string(id);
			ImGui::DragFloat(rotateSpeedStr.c_str(), &spinSpeed, 1.0f, -50, 50);
		}
		
		if (ImGui::CollapsingHeader("Model"))
		{
			string ModelStr = "Model##" + to_string(id);
			if (ImGui::BeginCombo(ModelStr.c_str(), modelName.c_str()))
			{
				for (auto m : *ModelManager::Resources())
				{
					const bool is_selected = (m.second == model);
					if (ImGui::Selectable(m.first.c_str(), is_selected))
					{
						model = ModelManager::GetModel(m.first);
						modelName = m.first;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

			}

			// Animation information.
			if (model != nullptr && model->boneStructure != nullptr)
			{
				string boneStr = "Selected Bone##" + to_string(id);
				ImGui::DragInt(boneStr.c_str(), &selectedBone, 0.1, 0, model->boneStructure->numBones);

				if (model->animations.size() > 0)
				{
					string animationNameStr = "Animation##" + to_string(id);
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

					string AnimSpeedStr = "Animation Speed##" + to_string(id);
					ImGui::DragFloat(AnimSpeedStr.c_str(), &animationSpeed, 0.1f, -2, 2);
					
					string AnimLoopStr = "Loop##" + to_string(id);
					ImGui::Checkbox(AnimLoopStr.c_str(), &loopAnimation);
					ImGui::SameLine();
					string AnimPlayStr = playAnimation ? "Pause##" + to_string(id) : "Play##" + to_string(id);
					if (ImGui::Button(AnimPlayStr.c_str())) playAnimation = !playAnimation;

					string animTimeStr = "Animation Time##" + to_string(id);
					ImGui::SliderFloat(animTimeStr.c_str(), &animationTime, 0, model->animations[selectedAnimation]->duration);
				}
			}
		

			// Mesh node hierarchy
			if (model != nullptr && model->childNodes != nullptr)
			{
				model->childNodes->DrawGUISimple();
			}

			string textureStr = "Texture##" + to_string(id);
			if (ImGui::BeginCombo(textureStr.c_str(), textureName.c_str()))
			{
				for (auto t : *TextureManager::Textures())
				{
					const bool is_selected = (t.second == texture);
					if (ImGui::Selectable(t.first.c_str(), is_selected))
					{
						texture = TextureManager::GetTexture(t.first);
						textureName = t.first;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			string shaderStr = "Shader##" + to_string(id);
			if (ImGui::BeginCombo(shaderStr.c_str(), shaderName.c_str()))
			{
				for (auto s : *ShaderManager::ShaderPrograms())
				{
					const bool is_selected = (s.second == shader);
					if (ImGui::Selectable(s.first.c_str(), is_selected))
					{
						shader = ShaderManager::GetShaderProgram(s.first);
						shaderName = s.first;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			string materialStr = "Material##" + to_string(id);
			if (ImGui::BeginCombo(materialStr.c_str(), materialName.c_str()))
			{
				for (auto m : *MaterialManager::Materials())
				{
					const bool is_selected = (m.second == material);
					if (ImGui::Selectable(m.first.c_str(), is_selected))
					{
						material = MaterialManager::GetMaterial(m.first);
						materialName = m.first;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		
			if (material)
			{
				ImGui::Indent();
				if (ImGui::CollapsingHeader("Material"))
				{
					material->DrawGUI();
				}
			}
		}

		int childCount = (int)children.size();
		string childrenString = "Children (" + to_string(childCount) + ")##" + to_string(id);
		if (ImGui::CollapsingHeader(childrenString.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			for (auto c : children)
				c->DrawGUI();
		}
		

		ImGui::TreePop();
	}
}

void Object::DrawGUISimple()
{
	string idStr = to_string(id);
	string objectStr = objectName + "##" + idStr;
	if (ImGui::TreeNode(objectStr.c_str()))
	{
		if (ImGui::CollapsingHeader("Transform"))
		{
			string positionStr = "Pos##" + to_string(id);
			ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]);

			string rotationStr = "Rot##" + to_string(id);
			ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180);

			string scaleStr = "Scale##" + to_string(id);
			ImGui::DragFloat3(scaleStr.c_str(), &localScale[0]);

			//ImGui::BeginDisabled();
			/*ImGui::InputFloat4("X", &transform[0].x);
			ImGui::InputFloat4("Y", &transform[1].x);
			ImGui::InputFloat4("Z", &transform[2].x);
			ImGui::InputFloat4("T", &transform[3].x);
			ImGui::InputFloat4("lX", &localTransform[0].x);
			ImGui::InputFloat4("lY", &localTransform[1].x);
			ImGui::InputFloat4("lZ", &localTransform[2].x);
			ImGui::InputFloat4("lT", &localTransform[3].x);*/
			//ImGui::EndDisabled();
		}

		int childCount = (int)children.size();
		string childrenString = "Children (" + to_string(childCount) + ")##" + to_string(id);
		if (ImGui::CollapsingHeader(childrenString.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			for (auto c : children)
				c->DrawGUISimple();
		}
		ImGui::TreePop();
	}
}

void Object::AddChild(Object* child)
{
	child->parent = this;
	children.push_back(child);
}

void Object::CleanUpChildren()
{
	for (int i = 0; i < children.size(); i++)
	{
		if (children[i]->markedForDeletion)
		{
			children[i]->DeleteAllChildren();
			children.erase(children.begin() + i);
			i--;
		}
		else
			children[i]->CleanUpChildren();
	}
}

void Object::DeleteAllChildren()
{
	for (auto c : children)
		c->DeleteAllChildren();

	children.clear();
}

void Object::Write(std::ostream& out)
{
	FileUtils::WriteString(out, objectName);
	
	FileUtils::WriteVec(out, localPosition);
	FileUtils::WriteVec(out, localRotation);
	FileUtils::WriteVec(out, localScale);

	FileUtils::WriteBool(out, spin);
	FileUtils::WriteFloat(out, spinSpeed);
	
	FileUtils::WriteString(out, modelName);

	FileUtils::WriteInt(out, selectedAnimation);
	FileUtils::WriteString(out, animationName);
	FileUtils::WriteFloat(out, animationSpeed);

	FileUtils::WriteString(out, textureName);
	FileUtils::WriteString(out, shaderName);
	FileUtils::WriteString(out, materialName);

	// write children
	int numChildren = (int)children.size();
	FileUtils::WriteInt(out, numChildren);
	for (int i = 0; i < numChildren; i++)
		children[i]->Write(out);
}

void Object::Read(std::istream& in)
{
	FileUtils::ReadString(in, objectName);
	
	FileUtils::ReadVec(in, localPosition);
	FileUtils::ReadVec(in, localRotation);
	FileUtils::ReadVec(in, localScale);

	FileUtils::ReadBool(in, spin);
	FileUtils::ReadFloat(in, spinSpeed);

	FileUtils::ReadString(in, modelName);
	model = ModelManager::GetModel(modelName);

	FileUtils::ReadInt(in, selectedAnimation);
	FileUtils::ReadString(in, animationName);
	FileUtils::ReadFloat(in, animationSpeed);

	FileUtils::ReadString(in, textureName);
	texture = TextureManager::GetTexture(textureName);
	FileUtils::ReadString(in, shaderName);
	shader = ShaderManager::GetShaderProgram(shaderName);
	FileUtils::ReadString(in, materialName);
	material = MaterialManager::GetMaterial(materialName);

	// read children
	int numChildren;
	FileUtils::ReadInt(in, numChildren);
	for (int i = 0; i < numChildren; i++)
	{
		auto o = Scene::CreateObject(this);
		o->Read(in);
	}
}

void Object::UpdateBoneMatrixBuffer(float frameTime)
{
	ProcessNode(frameTime, selectedAnimation, model->childNodes, mat4(1));
}

void Object::ProcessNode(float frameTime, int animationIndex, Object* node, mat4 accumulated)
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
			Model::Animation::AnimationKey &key = channel->second.keys[frameIndex];
			Model::Animation::AnimationKey &nextKey = channel->second.keys[nextFrameIndex];

			// Apply transformation.
			mat4 scale = glm::scale(glm::mat4(1), glm::mix(key.scale, nextKey.scale, t));				// generate mixed scale matrix		
			mat4 rotate = glm::mat4_cast(glm::slerp(key.rotation,nextKey.rotation, t));					// generate mixed rotation matrix
			mat4 translate = glm::translate(glm::mat4(1), glm::mix(key.position, nextKey.position, t));	// generate mixed translation matrix
			nodeTransformation = translate * rotate * scale;											// combine
		}
	}
	
	mat4 globalTransform = accumulated * nodeTransformation;											// Apply matrix to accumulated transform down the tree.

	// if it was an actual bone - apply it the transform buffer that gets sent to the vertex shader.
	if (bufferIndex != model->boneStructure->boneMapping.end())
		boneTransforms[bufferIndex->second] = globalTransform * model->boneStructure->boneInfo[bufferIndex->second].offset;

	// Process children.
	for (auto c : node->children)
		ProcessNode(frameTime, animationIndex, c, globalTransform);

}

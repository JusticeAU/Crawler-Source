#include "Object.h"
#include <string>
#include "ShaderProgram.h"
#include "Camera.h"
#include <string>
#include "Scene.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "FileUtils.h"

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

	meshName = "";
	mesh = nullptr;

	textureName = "";
	texture = nullptr;

	shaderName = "";
	shader = nullptr;
}

void Object::Update(float delta)
{
	// Move the cube by an offset
	if (parent)
	{
		transform =
			parent->transform
			* localTransform;
	}
	else
	{
		transform = localTransform;
	}

	for (auto c : children)
		c->Update(delta);

}

void Object::Draw()
{
	if (mesh == nullptr || shader == nullptr || texture == nullptr)
		return;

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

	// Texture Uniforms
	texture->Bind(1);
	shader->SetIntUniform("diffuseTex", 1);

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


	// Draw triangle
	glBindVertexArray(mesh->vao);

	// check if we're using index buffers on this mesh by hecking if indexbufferObject is valid (was it set up?)
	if (mesh->ibo != 0) // Draw with index buffering
		glDrawElements(GL_TRIANGLES, 3 * mesh->tris, GL_UNSIGNED_INT, 0);
	else // draw simply.
		glDrawArrays(GL_TRIANGLES, 0, 3 * mesh->tris);

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
			Object* spawn = Scene::CreateObject(this);

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
			ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]);

			string rotationStr = "Rot##" + to_string(id);
			ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180);

			string scaleStr = "Scale##" + to_string(id);
			ImGui::DragFloat3(scaleStr.c_str(), &localScale[0]);

			//ImGui::BeginDisabled();
			ImGui::InputFloat4("X", &transform[0].x);
			ImGui::InputFloat4("Y", &transform[1].x);
			ImGui::InputFloat4("Z", &transform[2].x);
			ImGui::InputFloat4("T", &transform[3].x);
			ImGui::InputFloat4("lX", &localTransform[0].x);
			ImGui::InputFloat4("lY", &localTransform[1].x);
			ImGui::InputFloat4("lZ", &localTransform[2].x);
			ImGui::InputFloat4("lT", &localTransform[3].x);
			//ImGui::EndDisabled();
		}
		
		if (ImGui::CollapsingHeader("Model"))
		{
			string meshStr = "Mesh##" + to_string(id);
			if (ImGui::BeginCombo(meshStr.c_str(), meshName.c_str()))
			{
				for (auto m : *MeshManager::Meshes())
				{
					const bool is_selected = (m.second == mesh);
					if (ImGui::Selectable(m.first.c_str(), is_selected))
					{
						mesh = MeshManager::GetMesh(m.first);
						meshName = m.first;
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// Mesh node hierarchy
			if (mesh != nullptr && mesh->childNodes.size() > 0)
			{
				mesh->childNodes[0]->parent = this;
				mesh->childNodes[0]->DrawGUISimple();
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

		int childCount = children.size();
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
	mesh = MeshManager::GetMesh("_cube");
	shader = ShaderManager::GetShaderProgram("shaders/phong");
	texture = TextureManager::GetTexture("models/uv_test.tga");
	Update(0.0f);
	Draw();

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
			ImGui::InputFloat4("X", &transform[0].x);
			ImGui::InputFloat4("Y", &transform[1].x);
			ImGui::InputFloat4("Z", &transform[2].x);
			ImGui::InputFloat4("T", &transform[3].x);
			ImGui::InputFloat4("lX", &localTransform[0].x);
			ImGui::InputFloat4("lY", &localTransform[1].x);
			ImGui::InputFloat4("lZ", &localTransform[2].x);
			ImGui::InputFloat4("lT", &localTransform[3].x);
			//ImGui::EndDisabled();
		}

		int childCount = children.size();
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
	
	FileUtils::WriteString(out, meshName);
	FileUtils::WriteString(out, textureName);
	FileUtils::WriteString(out, shaderName);
	FileUtils::WriteString(out, materialName);

	// write children
	int numChildren = children.size();
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

	FileUtils::ReadString(in, meshName);
	mesh = MeshManager::GetMesh(meshName);
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

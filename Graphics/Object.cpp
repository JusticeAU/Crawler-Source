#include "Object.h"
#include <string>
#include "ShaderProgram.h"
#include "Camera.h"
#include <string>
#include "Scene.h"
#include "MeshManager.h"
#include "TextureManager.h"

using std::to_string;

Object::Object(int objectID)
{
	id = objectID;
	localPosition = { 0,0,0 };
	localRotation = { 0,0,0 };
	localScale = { 1,1,1 };
	transform = mat4(1);

	shader = new ShaderProgram();
	shader->LoadFromFiles("shaders\\passthrough.VERT", "shaders\\passthrough.FRAG");

	meshName = "_cube";
	mesh = MeshManager::GetMesh(meshName);

	textureName = "models/numbered_grid.tga";
	texture = TextureManager::GetTexture(textureName);
}

void Object::Update(float delta)
{
	// Move the cube by an offset
	if (parent)
	{
		transform =
			parent->transform
			* glm::translate(glm::mat4(1), localPosition)
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.z), glm::vec3{ 0,0,1 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.y), glm::vec3{ 0,1,0 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.x), glm::vec3{ 1,0,0 })
			* glm::scale(glm::mat4(1), localScale);
	}
	else
	{
		transform = glm::translate(glm::mat4(1), localPosition)
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.z), glm::vec3{ 0,0,1 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.y), glm::vec3{ 0,1,0 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.x), glm::vec3{ 1,0,0 })
			* glm::scale(glm::mat4(1), localScale);
	}

	for (auto c : children)
		c->Update(delta);

}

void Object::Draw()
{

	// Combine the matricies
	glm::mat4 pvm = Camera::s_instance->GetMatrix() * transform;

	shader->Bind();
	shader->SetMatrixUniform("transformMatrix", pvm);
	shader->SetMatrixUniform("mMatrix", transform);
	shader->SetVectorUniform("lightDirection", glm::normalize(glm::vec3(0, -1, 0)));
	texture->Bind(1);
	shader->SetIntUniform("diffuseTex", 1);

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
	string objectStr = "Object (" + idStr + ")";
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
		
		
		if (ImGui::CollapsingHeader("Transform"))
		{
			string positionStr = "Pos##" + to_string(id);
			ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]);

			string rotationStr = "Rot##" + to_string(id);
			ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180);

			string scaleStr = "Scale##" + to_string(id);
			ImGui::DragFloat3(scaleStr.c_str(), &localScale[0]);
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
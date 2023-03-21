#include "Object.h"
#include <string>
#include "ShaderProgram.h"
#include "Camera.h"
#include <string>
#include "Scene.h"

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

	mesh = new Mesh();
	mesh->InitialiseCube();
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
	if (ImGui::TreeNode(idStr.c_str()))
	{
		ImGui::Text("Transform");
		string positionStr = "Pos##" + to_string(id);
		ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]);

		string rotationStr = "Rot##" + to_string(id);
		ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180);

		string scaleStr = "Scale##" + to_string(id);
		ImGui::DragFloat3(scaleStr.c_str(), &localScale[0]);


		string deleteStr = "Delete##" + to_string(id);
		if (ImGui::Button(deleteStr.c_str()))
			markedForDeletion = true;
		ImGui::SameLine();
		string childStr = "AddChild##" + to_string(id);
		if (ImGui::Button(childStr.c_str()))
			Object* spawn = Scene::CreateObject(this);
		ImGui::Text("Children");
		for (auto c : children)
			c->DrawGUI();

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
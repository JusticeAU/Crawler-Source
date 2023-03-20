#include "Object.h"
#include <string>
#include "ShaderProgram.h"
#include "Camera.h"
#include <string>

using std::to_string;

Object::Object()
{
	id = rand();
	localPosition = { 0,0,0 };
	localRotation = { 0,0,0 };

	shader = new ShaderProgram();
	shader->LoadFromFiles("shaders\\passthrough.VERT", "shaders\\passthrough.FRAG");

	mesh = new Mesh();
	mesh->InitialiseCube();
}

void Object::Update(float delta)
{
	// Move the cube by an offset
	if(parent)
		translationMat = glm::translate(glm::mat4(1), localPosition) * parent->translationMat;
	else
		translationMat = glm::translate(glm::mat4(1), localPosition);

	// Rotate the cube
	rotationMat = glm::rotate(glm::mat4(1), glm::radians(localRotation.x), glm::vec3{ 1,0,0 }) *
		glm::rotate(glm::mat4(1), glm::radians(localRotation.y), glm::vec3{ 0,1,0 }) *
		glm::rotate(glm::mat4(1), glm::radians(localRotation.z), glm::vec3{ 0,0,1 });

	for (auto c : children)
		c->Update(delta);

}

void Object::Draw()
{
	

	// Combine the matricies
	glm::mat4 transform = Camera::s_instance->GetMatrix() * translationMat * rotationMat;

	shader->Bind();
	shader->SetMatrixUniform("transformMatrix", transform);
	shader->SetMatrixUniform("mMatrix", rotationMat);
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
		string positionStr = "Pos##" + to_string(id);
		ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]);

		string rotationStr = "Rot##" + to_string(id);
		ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180);

		string deleteStr = "Delete##" + to_string(id);
		if (ImGui::Button(deleteStr.c_str()))
			markedForDeletion = true;

		string childStr = "AddChild##" + to_string(id);
		if (ImGui::Button(childStr.c_str()))
		{
			Object* spawn = new Object();
			spawn->parent = this;
			children.push_back(spawn);
		}

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

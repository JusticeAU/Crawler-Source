#include "Object.h"
#include <string>
#include "ShaderProgram.h"
#include "Camera.h"


Object::Object()
{
	id = rand();
	position = { 0,0,0 };

	shader = new ShaderProgram();
	shader->LoadFromFiles("shaders\\passthrough.VERT", "shaders\\passthrough.FRAG");

	mesh = new Mesh();
	mesh->InitialiseCube();
}

void Object::Update(float delta)
{
}

void Object::Draw()
{
	// Move the cube by an offset
	glm::mat4 translation = glm::translate(glm::mat4(1), position);

	// Rotate the cube
	glm::mat4 rotation = glm::rotate(glm::mat4(1), (float)glfwGetTime() * 0.2f, glm::normalize(glm::vec3(0.5f, 1, 1)));

	// Combine the matricies
	glm::mat4 transform = Camera::s_instance->GetMatrix() * translation * rotation;

	shader->Bind();
	shader->SetMatrixUniform("transformMatrix", transform);
	shader->SetMatrixUniform("mMatrix", rotation);
	shader->SetVectorUniform("lightDirection", glm::normalize(glm::vec3(0, -1, 0)));


	// Draw triangle
	glBindVertexArray(mesh->vao);


	// check if we're using index buffers on this mesh by hecking if indexbufferObject is valid (was it set up?)
	if (mesh->ibo != 0) // Draw with index buffering
		glDrawElements(GL_TRIANGLES, 3 * mesh->tris, GL_UNSIGNED_INT, 0);
	else // draw simply.
		glDrawArrays(GL_TRIANGLES, 0, 3 * mesh->tris);
}

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

	// Open GL Init
	glGenBuffers(1, &bufferID);

	// Enable depth testing (Also need ot make sure we are clearing depth values when calling screen clear)
	glEnable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(someFloats), someFloats, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// These align with the 'layout' keywords in the shader.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void Object::Update(float delta)
{
}

void Object::Draw()
{
	// Draw triangle
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);

	// glVertexAttribPointer - These align with the layout keyword in the shader
	// index, size, type, normalised, stride, location
	// index is the layout location in shader
	// size is how many units to read
	// type is the unit to read
	// normalised is: - something to do with matrix row column stuff.
	// stride is how far along to read everything for this vert?
	// location is the point within the stride for this particular attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 6));


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

	// type, start, number of verticies
	glDrawArrays(GL_TRIANGLES, 0, sizeof(someFloats) / sizeof(float) * 3);
}

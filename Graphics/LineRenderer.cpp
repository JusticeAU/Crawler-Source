#include "LineRenderer.h"
#include "ShaderManager.h"
#include "ShaderProgram.h"
void LineRenderer::Initialise()
{
	if (!s_instance)
	{
		s_instance = new LineRenderer();
		glGenBuffers(1, &s_instance->vertBufferID);
		glGenBuffers(1, &s_instance->colourBufferID);

		s_instance->shader = ShaderManager::GetShaderProgram("engine/shader/LineRenderer");
		glLineWidth(3.0f);
	}
}

void LineRenderer::DrawLine(glm::vec3 a, glm::vec3 b, glm::vec3 colour)
{
	s_instance->verts.push_back(a);
	s_instance->verts.push_back(b);
	s_instance->colours.push_back(colour);
	s_instance->colours.push_back(colour);
}

void LineRenderer::DrawFlatBox(glm::vec3 position, float size, glm::vec3 colour)
{
	DrawLine(position + glm::vec3(-size, size, 0), position + glm::vec3(size, size, 0), colour);
	DrawLine(position + glm::vec3(size, size, 0), position + glm::vec3(size, -size, 0), colour);
	DrawLine(position + glm::vec3(size, -size, 0), position + glm::vec3(-size, -size, 0), colour);
	DrawLine(position + glm::vec3(-size, -size, 0), position + glm::vec3(-size, size, 0), colour);
}

void LineRenderer::Render(glm::mat4 pvMatrix)
{
	// Test if there is anything to draw.
	if (s_instance->verts.size() == 0)
		return;

	glBindVertexArray(0);
	// Configure shader
	s_instance->shader->Bind();
	s_instance->shader->SetMatrixUniform("pvMatrix", pvMatrix);

	// Prepare and upload line+Colour data.
	glDisable(GL_DEPTH_TEST);
	glBindBuffer(GL_ARRAY_BUFFER, s_instance->vertBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * s_instance->verts.size(), s_instance->verts.data(), GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, s_instance->colourBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * s_instance->colours.size(), s_instance->colours.data(), GL_STREAM_DRAW);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	// Draw
	glDrawArrays(GL_LINES, 0, s_instance->verts.size());
	
	// Clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	s_instance->verts.clear();
	s_instance->colours.clear();
}

LineRenderer* LineRenderer::s_instance = nullptr;
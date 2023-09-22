#include "PostProcess.h"
#include "FrameBuffer.h"
#include "Window.h"
#include "MeshManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "SceneRenderer.h"
#include "ComponentCamera.h"

PostProcess::PostProcess(string name) : m_name(name)
{
	m_frameBuffer = new FrameBuffer(FrameBuffer::Type::PostProcess);
	TextureManager::s_instance->AddFrameBuffer(name.c_str(), m_frameBuffer);
	
	if (s_passthroughShader == nullptr)
		Init();
}

PostProcess::~PostProcess()
{
	TextureManager::s_instance->RemoveFrameBuffer(m_name.c_str());
	delete m_frameBuffer;
}

void PostProcess::BindFrameBuffer()
{
	m_frameBuffer->BindTarget();
}

void PostProcess::Process(ComponentCamera* camera)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST); // May not need to do this?

	m_shader->Bind();
	m_shader->SetIntUniform("frame", m_textureBindPoint); // previous post process step would have bound the texture.
	// TODO Abstract these in to post process meta data stuff. Don't particularly want to hard-code different post process types. WIll need to ponder.
	
	// Fade Colour shader
	m_shader->SetVector3Uniform("fadeColour", camera->postProcessFadeColour);
	m_shader->SetFloatUniform("fadeAmount", camera->postProcessFadeAmount);

	// Draw the frame quad
	glBindVertexArray(s_frame->vao);

	// check if we're using index buffers on this mesh by checking if indexbufferObject is valid (was it set up?)
	if (s_frame->ibo != 0) // Draw with index buffering
		glDrawElements(GL_TRIANGLES, 3 * s_frame->tris, GL_UNSIGNED_INT, 0);
	else // draw simply.
		glDrawArrays(GL_TRIANGLES, 0, 3 * s_frame->tris);

	glEnable(GL_DEPTH_TEST);

}

void PostProcess::BindOutput()
{
	m_frameBuffer->BindTexture(m_textureBindPoint);
}

// Suppler a shader or a standard passthrough shader will be used.
void PostProcess::PassThrough(bool ownShaderBound)
{
	// check that the reference has been initialised first
	if (s_passthroughShader == nullptr)
		Init();

	glDisable(GL_DEPTH_TEST);
	if (!ownShaderBound)
	{
		s_passthroughShader->Bind();
		s_passthroughShader->SetIntUniform("frame", 20);
	}
	
	// Draw the frame quad
	glBindVertexArray(s_frame->vao);

	// check if we're using index buffers on this mesh by checking if indexbufferObject is valid (was it set up?)
	if (s_frame->ibo != 0) // Draw with index buffering
		glDrawElements(GL_TRIANGLES, 3 * s_frame->tris, GL_UNSIGNED_INT, 0);
	else // draw simply.
		glDrawArrays(GL_TRIANGLES, 0, 3 * s_frame->tris);
	glEnable(GL_DEPTH_TEST);
}

void PostProcess::Init()
{
	s_frame = MeshManager::GetMesh("_fsQuad");
	s_passthroughShader = ShaderManager::GetShaderProgram("engine/shader/postProcess/passThrough");
}

Mesh* PostProcess::s_frame = nullptr;
ShaderProgram* PostProcess::s_passthroughShader = nullptr;
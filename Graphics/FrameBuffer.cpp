#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Window.h"
#include "Scene.h"
#include "SceneRenderer.h"
#include "LogUtils.h"

using std::vector;

FrameBuffer::FrameBuffer(Type type)
{
	m_type = type;
	m_texture = new Texture();
	m_texture->loaded = true;
	// generate frame buffer, texture buffer and depth buffer.

	// Generate the texture
	switch (type)
	{
	case Type::CameraTargetMultiSample:
	{
		glGenFramebuffers(1, &m_fbID);
		glGenTextures(1, &m_texID);
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texID);
		glm::ivec2 res = Window::GetViewPortSize();
		m_width = res.x;
		m_height = res.y;
		m_isScreenBuffer = true;

		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples, GL_RGBA, res.x, res.y, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Generate the depth stencil
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_depthID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples, GL_DEPTH_STENCIL, res.x, res.y, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Link
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_texID, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthID, 0);

		// unbind.
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	}
	case Type::CameraTargetSingleSample:
	{
		glGenFramebuffers(1, &m_fbID);
		glGenTextures(1, &m_texID);
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_2D, m_texID);
		glm::ivec2 res = Window::GetViewPortSize();
		m_width = res.x;
		m_height = res.y;
		m_isScreenBuffer = true;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.x, res.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Generate the depth stencil
		glBindTexture(GL_TEXTURE_2D, m_depthID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, res.x, res.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Link
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texID, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);

		// unbind.
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	}
	case Type::PostProcess:
	{
		glGenFramebuffers(1, &m_fbID);
		glGenTextures(1, &m_texID);
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_2D, m_texID);
		glm::ivec2 res = Window::GetViewPortSize();
		m_width = res.x;
		m_height = res.y;
		m_isScreenBuffer = true;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.x, res.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Link
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texID, 0);

		// unbind.
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	}
	case Type::ObjectPicker:
	{
		glGenFramebuffers(1, &m_fbID);
		glGenTextures(1, &m_texID);
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_2D, m_texID);
		glm::ivec2 res = Window::GetViewPortSize();
		m_width = res.x;
		m_height = res.y;
		m_isScreenBuffer = true;

		// generate frame buffer, texture buffer and depth buffer.
		glGenFramebuffers(1, &m_fbID);
		glGenTextures(1, &m_texID);
		glGenTextures(1, &m_depthID);

		// Generate the texture
		glBindTexture(GL_TEXTURE_2D, m_texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, res.x, res.y, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Generate the depth stencil
		glBindTexture(GL_TEXTURE_2D, m_depthID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, res.x, res.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Link
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texID, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);

		// unbind.
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	}
	case Type::ShadowMap:
	{
		glGenFramebuffers(1, &m_fbID);
		glGenTextures(1, &m_texID);
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_2D, m_texID);
		m_width = 4096;
		m_height = 4096;

		// Generate the depth map and store in m_depthID
		glBindTexture(GL_TEXTURE_2D, m_depthID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Link
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);
		
		// Tell OpenGL explicitly that we're not rendering any colour data.
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		
		// unbind.
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	}
	case Type::ShadowCubeMap:
	{

		m_width = 512;
		m_height = 512;
		// Create the FBO.
		glGenFramebuffers(1, &m_fbID); 

		// Create Cube Depth Map
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_depthID);
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);

		// Disable writes to the color buffer
		glDrawBuffer(GL_NONE);
		// Disable reads from the color buffer
		glReadBuffer(GL_NONE);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (Status != GL_FRAMEBUFFER_COMPLETE) {
			printf("FB error, status: 0x%x\n", Status);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		break;
	}
	case Type::SSAOgBuffer:
	{
		glm::ivec2 res = Window::GetViewPortSize();
		m_width = res.x;
		m_height = res.y;
		m_isScreenBuffer = true;

		glGenFramebuffers(1, &m_fbID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
		// position color buffer
		glGenTextures(1, &m_gPosition);
		glBindTexture(GL_TEXTURE_2D, m_gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);
		// normal color buffer
		glGenTextures(1, &m_gNormal);
		glBindTexture(GL_TEXTURE_2D, m_gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);
		
		// color + specular color buffer - We use a simplified gBuffer for SSAO & DepthPrePass only, don't need this right now, we are still forwardish rendered.
		/*
		glGenTextures(1, &m_gAlbedo);
		glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedo, 0);
		*/

		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(2, attachments); // Note the 2 here, we're not using the albedo buffer.
		
		// create and attach depth buffer (renderbuffer) - old method from tutorial, using a depth stencil texture for consistency with other buffers.
		/*glGenRenderbuffers(1, &m_rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);*/

		// Generate the depth stencil
		glGenTextures(1, &m_depthID);
		glBindTexture(GL_TEXTURE_2D, m_depthID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, res.x, res.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LogUtils::Log("Framebuffer not complete!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	}
	case Type::SSAOColourBuffer:
	{
		glm::ivec2 res = Window::GetViewPortSize();
		m_width = res.x;
		m_height = res.y;
		m_isScreenBuffer = true;

		glGenFramebuffers(1, &m_fbID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);

		glGenTextures(1, &m_texID);
		glBindTexture(GL_TEXTURE_2D, m_texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		// clamp to edge to avoid weird behaviour at edge of screen
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texID, 0);
		break;
	}
	}

	m_texture->texID = m_texID;
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &m_fbID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
}

void FrameBuffer::BindTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
	glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::BindTarget(GLenum cubeFace)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeFace, m_depthID, 0);
	glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::BindTexture(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	if(m_type == Type::CameraTargetMultiSample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texID);
	else if(m_type == Type::ShadowMap)
		glBindTexture(GL_TEXTURE_2D, m_depthID);
	else if(m_type == Type::ShadowCubeMap)
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_depthID);
	else
		glBindTexture(GL_TEXTURE_2D, m_texID);
}

void FrameBuffer::BindDepth(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, m_depthID);
}

void FrameBuffer::BindAsDepthAttachment()
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);
}

void FrameBuffer::BindGPosition(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, m_gPosition);
}

void FrameBuffer::BindGNormal(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, m_gNormal);
}

void FrameBuffer::BindGAlbedo(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
}

void FrameBuffer::BindRBODepth(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, m_rboDepth);
}

void FrameBuffer::UnBindTexture(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::Resize()
{
	if (m_primaryTarget)
		m_type = SceneRenderer::msaaEnabled ? Type::CameraTargetMultiSample : Type::CameraTargetSingleSample;

	FrameBuffer* newFB = new FrameBuffer(m_type);
	glDeleteFramebuffers(1, &m_fbID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
	glDeleteTextures(1, &m_gPosition);
	glDeleteTextures(1, &m_gAlbedo);
	glDeleteTextures(1, &m_gNormal);
	m_fbID = newFB->m_fbID;
	m_texID = newFB->m_texID;
	m_depthID = newFB->m_depthID;
	m_gPosition = newFB->m_gPosition;
	m_gAlbedo = newFB->m_gAlbedo;
	m_gNormal = newFB->m_gNormal;

	
	m_width = newFB->m_width;
	m_height = newFB->m_height;

	m_texture->texID = newFB->m_texture->texID;
}

unsigned int FrameBuffer::GetObjectID(int x, int y)
{
	if (m_type != Type::ObjectPicker)
		return 0;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbID);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	float pixel[3];
	glReadPixels(x, m_height - y, 1, 1, GL_RGB, GL_FLOAT, &pixel);

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return (unsigned int)pixel[0];
}

void FrameBuffer::SetMSAASampleLevels(int samples)
{
	m_MSAASamples = samples;
}

int FrameBuffer::m_MSAASamples = 4;
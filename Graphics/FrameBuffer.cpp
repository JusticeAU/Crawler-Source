#include "FrameBuffer.h"
#include "Texture.h"
#include "Window.h"

using std::vector;

FrameBuffer::FrameBuffer(Type type)
{
	m_type = type;
	m_texture = new Texture();
	// generate frame buffer, texture buffer and depth buffer.
	glGenFramebuffers(1, &m_fbID);
	glGenTextures(1, &m_texID);
	glGenTextures(1, &m_depthID);

	// Generate the texture
	glBindTexture(GL_TEXTURE_2D, m_texID);
	switch (type)
	{
	case Type::CameraTarget:
	{
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
		break;
	}
	case Type::PostProcess:
	{
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
		break;
	}
	case Type::ObjectPicker:
	{
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
		break;
	}
	case Type::ShadowMap:
	{
		m_width = 1024;
		m_height = 1024;

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
		break;
	}
	}
	
	// unbind.
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

void FrameBuffer::BindTexture(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	if(m_type == Type::ShadowMap)
		glBindTexture(GL_TEXTURE_2D, m_depthID);
	else
		glBindTexture(GL_TEXTURE_2D, m_texID);
}

void FrameBuffer::UnBindTexture(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::Resize()
{
	FrameBuffer* newFB = new FrameBuffer(m_type);
	glDeleteFramebuffers(1, &m_fbID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
	m_fbID = newFB->m_fbID;
	m_texID = newFB->m_texID;
	m_depthID = newFB->m_depthID;
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

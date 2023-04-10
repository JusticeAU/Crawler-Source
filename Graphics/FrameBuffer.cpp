#include "FrameBuffer.h"
#include "Texture.h"

FrameBuffer::FrameBuffer(int width, int height) : m_width(width), m_height(height)
{
	m_texture = new Texture();
	// generate frame buffer, texture buffer and depth buffer.
	glGenFramebuffers(1, &m_fbID);
	glGenTextures(1, &m_texID);
	glGenTextures(1, &m_depthID);

	// Generate the texture
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Generate the depth stencil
	glBindTexture(GL_TEXTURE_2D, m_depthID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, m_width, m_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// unbind.
	glBindTexture(GL_TEXTURE_2D, 0);

	// Link
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbID);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texID, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthID, 0);

	// Unbind
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
	glBindTexture(GL_TEXTURE_2D, m_texID);
}

void FrameBuffer::UnBindTexture(int texture)
{
	glActiveTexture(GL_TEXTURE0 + texture);
	glBindTexture(GL_TEXTURE_2D, 0);
}

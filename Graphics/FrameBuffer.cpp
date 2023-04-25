#include "FrameBuffer.h"
#include "Texture.h"

using std::vector;

FrameBuffer::FrameBuffer(int width, int height, bool screenBuffer) : m_width(width), m_height(height), m_isScreenBuffer(screenBuffer)
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

void FrameBuffer::Resize(int width, int height)
{
	FrameBuffer* newFB = new FrameBuffer(width, height);
	glDeleteFramebuffers(1, &m_fbID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
	m_fbID = newFB->m_fbID;
	m_texID = newFB->m_texID;
	m_depthID = newFB->m_depthID;
	m_width = width;
	m_height = height;

	m_texture->texID = newFB->m_texture->texID;

	if (m_isObjectPicker) // This is bogus. do it properly once working.
		MakeObjectPicker();
}

void FrameBuffer::MakeObjectPicker()
{
	m_isObjectPicker = true;
	glDeleteFramebuffers(1, &m_fbID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);

	// generate frame buffer, texture buffer and depth buffer.
	glGenFramebuffers(1, &m_fbID);
	glGenTextures(1, &m_texID);
	glGenTextures(1, &m_depthID);

	// Generate the texture
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);
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


unsigned int FrameBuffer::GetObjectID(int x, int y)
{
	if (!m_isObjectPicker)
		return 6969;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbID);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	float pixel[3];
	glReadPixels(x, m_height - y, 1, 1, GL_RGB, GL_FLOAT, &pixel);

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return (unsigned int)pixel[0];
}

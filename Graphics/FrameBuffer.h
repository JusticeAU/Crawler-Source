#pragma once
#include "Graphics.h"

class Texture;

class FrameBuffer
{
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();

	FrameBuffer(const FrameBuffer&) = delete;
	const FrameBuffer& operator=(const FrameBuffer& other) = delete;

	void BindTarget();
	static void UnBindTarget() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
	void BindTexture(int texture);
	static void UnBindTexture(int texture);
	Texture* GetTexture() { return m_texture; }
protected:
	GLuint m_fbID;
	GLuint m_texID;
	GLuint m_depthID;
	int m_width, m_height;
	Texture* m_texture;
};


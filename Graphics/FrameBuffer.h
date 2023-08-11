#pragma once
#include "Graphics.h"
#include <vector>

class Texture;

class FrameBuffer
{
public:
	enum class Type
	{
		CameraTargetMSAA,	// rgba, depth/stencil buffer, viewport resolution, with MultiSampling
		CameraTargetBlit,	// rgba, depth/stencil buffer, viewport resolution.
		PostProcess,	// same as above but no depth/stencil buffer
		ObjectPicker,	// rgb32f, has depth buffer, viewport resolution.
		ShadowMap
	};

public:
	FrameBuffer(Type type);
	
	~FrameBuffer();

	FrameBuffer(const FrameBuffer&) = delete;
	const FrameBuffer& operator=(const FrameBuffer& other) = delete;

	void BindTarget();
	static void UnBindTarget() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
	void BindTexture(int texture);
	static void UnBindTexture(int texture);
	static void SetMSAASampleLevels(int samples);

	Type GetType() { return m_type; }

	Texture* GetTexture() { return m_texture; }
	GLuint GetID() { return m_fbID; }
	GLuint GetTextureID() { return m_texID; }

	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }


	const bool isScreenBuffer() { return m_isScreenBuffer; }

	void Resize(); // Resizes to view port size

	unsigned int GetObjectID(int x, int y);
protected:
	Type m_type;
	GLuint m_fbID;
	GLuint m_texID;
	GLuint m_depthID;
	int m_width = 0;
	int m_height = 0;
	Texture* m_texture;

	bool m_isScreenBuffer = false; // maybe could type these with a bitset?

	static int m_MSAASamples;
};


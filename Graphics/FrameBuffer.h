#pragma once
#include "Graphics.h"
#include <vector>

class Texture;

class FrameBuffer
{
public:
	enum class Type
	{
		CameraTargetMultiSample,	// rgba, depth/stencil buffer, viewport resolution, with MultiSampling
		CameraTargetSingleSample,	// rgba, depth/stencil buffer, viewport resolution.
		PostProcess,	// same as above but no depth/stencil buffer
		ObjectPicker,	// rgb32f, has depth buffer, viewport resolution.
		ShadowMap,
		ShadowCubeMap,
		gBuffer,
		SSAOColourBuffer
	};


public:
	FrameBuffer(Type type, float scale = 1.0f);

	~FrameBuffer();

	FrameBuffer(const FrameBuffer&) = delete;
	const FrameBuffer& operator=(const FrameBuffer& other) = delete;

	void BindTarget();
	void BindTarget(GLenum cubeFace);
	static void UnBindTarget() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	void BindTexture(int texture);
	void BindEmission(int texture);
	void BindDepth(int texture);
	void BindAsDepthAttachment();
	void BindGPosition(int texture);
	void BindGNormal(int texture);
	void BindGAlbedo(int texture);
	void BindRBODepth(int texture);

	static void UnBindTexture(int texture);
	static void SetMSAASampleLevels(int samples);

	Type GetType() { return m_type; }

	Texture* GetTexture() { return m_texture; }
	GLuint GetID() { return m_fbID; }
	GLuint GetTextureID() { return m_texID; }

	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }


	const bool isScreenBuffer() { return m_isScreenBuffer; }
	const bool isPrimaryTarget() { return m_primaryTarget; }
	void MakePrimaryTarget() { m_primaryTarget = true; }

	void Resize(); // Resizes to view port size

	unsigned int GetObjectID(int x, int y);
public:
	bool m_primaryTarget = false; // if true MSAA being true or false will decide what FBO type this target gets and how it behaves in render passes.
	Type m_type;
	float m_scale;
	GLuint m_fbID;
	GLuint m_texID;
	GLuint m_depthID;
	
	// PBR MRT
	GLuint m_emissiveTexID;

	// SSAO bBuffer stuff
	GLuint m_gPosition;
	GLuint m_gNormal;
	GLuint m_gAlbedo;
	GLuint m_rboDepth;

	int m_width = 0;
	int m_height = 0;
	Texture* m_texture;

	bool m_isScreenBuffer = false; // maybe could type these with a bitset?

	static int m_MSAASamples;
};


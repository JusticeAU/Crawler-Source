#pragma once
#include "Graphics.h"

// stores ID of a uniform buffer and provides helper functions to manipulate it.
// Currently only used for skinned mesh rendering to store an array of bone transforms
// SubData functionality not required, and the size of the buffer is not going
// to change after it's initialised so OK for now.
// Locked as a GL_STATIC_DRAW type.
class UniformBuffer
{
public:
	UniformBuffer(unsigned int size);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer&) = delete;
	const UniformBuffer& operator=(const UniformBuffer& other) = delete;

	void Bind(const unsigned int index);
	void SendData(void* data);
protected:
	GLuint m_uboID;
	unsigned int m_size;
};


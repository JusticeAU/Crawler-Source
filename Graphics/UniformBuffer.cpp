#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(unsigned int size) : m_size(size)
{
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, m_size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers(0, &m_uboID);
}

// binds this particular buffer to the designated UNIFORM_BUFFER index.
void UniformBuffer::Bind(const unsigned int index)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, index, m_uboID);
}

// Upload new data in to the buffer. Assumes the data is the same size the buffer was initialised with.
void UniformBuffer::SendData(void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, m_size, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

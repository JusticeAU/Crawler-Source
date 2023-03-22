#include "Mesh.h"

void Mesh::Initialise(unsigned int vertCount, const Vertex* vertices, unsigned int indexCount, unsigned int* indices)
{
	// Check we havent already initialised a mesh. No handling for overwriting at this stage. Should just make a new mesh class.
	assert(vao == 0);

	// Generate buffers we'll need for thismesh
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, & vao);

	// Bind vertex array - binding is to make 'active' or 'current'
	glBindVertexArray(vao);

	// Bind the vertex buffer to the currently bound vertex array
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Upload our geometry data (array of Vertex structs)
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertCount, vertices, GL_STATIC_DRAW);

	// Tell the vao how our data is organised
	glEnableVertexAttribArray(0); // Vertex position  data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1); // Vertex colour data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colour));
	glEnableVertexAttribArray(2); // Normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(3); // UVs
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, uv));


	// bind indicies if there are any
	if (indexCount != 0)
	{
		glGenBuffers(1, &ibo);

		// bind vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		// fill vertex buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

		tris = indexCount / 3;
	}
	else {
		tris = vertCount / 3;
	}

	// Unbind buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),  0);
	glEnableVertexAttribArray(1); // Vertex colour data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
	glEnableVertexAttribArray(2); // Normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)24);

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

void Mesh::InitialiseCube()
{
	Mesh::Vertex vertices[36];
	#pragma region cube
	vertices[0].position = { -0.5, -0.5, -0.5 };
	vertices[0].normal = { 0,0,-1 };
	vertices[0].colour = { 0,0,1 };

	vertices[1].position = { -0.5, 0.5, -0.5 };
	vertices[1].normal = { 0,0,-1 };
	vertices[1].colour = { 0,0,1 };

	vertices[2].position = { 0.5, -0.5, -0.5 };
	vertices[2].normal = { 0,0,-1 };
	vertices[2].colour = { 0,0,1 };
	
	vertices[3].position = { -0.5, 0.5, -0.5 };
	vertices[3].normal = { 0,0,-1 };
	vertices[3].colour = { 0,0,1 };

	vertices[4].position = { 0.5, -0.5, -0.5 };
	vertices[4].normal = { 0,0,-1 };
	vertices[4].colour = { 0,0,1 };

	vertices[5].position = { 0.5, 0.5, -0.5 };
	vertices[5].normal = { 0,0,-1 };
	vertices[5].colour = { 0,0,1 };


	vertices[6].position = { -0.5, -0.5, 0.5 };
	vertices[6].normal = { 0,0,1 };
	vertices[6].colour = { 1, 1, 0 };

	vertices[7].position = { -0.5, 0.5, 0.5 };
	vertices[7].normal = { 0,0,1 };
	vertices[7].colour = { 1, 1, 0 };

	vertices[8].position = { 0.5, -0.5, 0.5 };
	vertices[8].normal = { 0,0,1 };
	vertices[8].colour = { 1, 1, 0 };

	vertices[9].position = { -0.5, 0.5, 0.5 };
	vertices[9].normal = { 0,0,1 };
	vertices[9].colour = { 1, 1, 0 };

	vertices[10].position = { 0.5, -0.5, 0.5 };
	vertices[10].normal = { 0,0,1 };
	vertices[10].colour = { 1, 1, 0 };

	vertices[11].position = { 0.5, 0.5, 0.5 };
	vertices[11].normal = { 0,0,1 };
	vertices[11].colour = { 1, 1, 0 };


	vertices[12].position = { -0.5, -0.5, -0.5 };
	vertices[12].normal = { 0,-1,0 };
	vertices[12].colour = { 0, 1, 0 };

	vertices[13].position = { -0.5, -0.5, 0.5 };
	vertices[13].normal = { 0,-1,0 };
	vertices[13].colour = { 0, 1, 0 };

	vertices[14].position = { 0.5, -0.5, -0.5 };
	vertices[14].normal = { 0,-1,0 };
	vertices[14].colour = { 0, 1, 0 };

	vertices[15].position = { -0.5, -0.5, 0.5 };
	vertices[15].normal = { 0,-1,0 };
	vertices[15].colour = { 0, 1, 0 };

	vertices[16].position = { 0.5, -0.5, -0.5 };
	vertices[16].normal = { 0,-1,0 };
	vertices[16].colour = { 0, 1, 0 };

	vertices[17].position = { 0.5, -0.5, 0.5 };
	vertices[17].normal = { 0,-1,0 };
	vertices[17].colour = { 0, 1, 0 };


	vertices[18].position = { -0.5, 0.5, -0.5 };
	vertices[18].normal = { 0,1,0 };
	vertices[18].colour = { 1, 0, 1 };

	vertices[19].position = { -0.5, 0.5, 0.5 };
	vertices[19].normal = { 0,1,0 };
	vertices[19].colour = { 1, 0, 1 };

	vertices[20].position = { 0.5, 0.5, -0.5 };
	vertices[20].normal = { 0,1,0 };
	vertices[20].colour = { 1, 0, 1 };

	vertices[21].position = { -0.5, 0.5, 0.5 };
	vertices[21].normal = { 0,1,0 };
	vertices[21].colour = { 1, 0, 1 };

	vertices[22].position = { 0.5, 0.5, -0.5 };
	vertices[22].normal = { 0,1,0 };
	vertices[22].colour = { 1, 0, 1 };

	vertices[23].position = { 0.5, 0.5, 0.5 };
	vertices[23].normal = { 0,1,0 };
	vertices[23].colour = { 1, 0, 1 };


	vertices[24].position = { -0.5, -0.5, -0.5 };
	vertices[24].normal = { -1,0,0 };
	vertices[24].colour = { 1, 0, 0 };

	vertices[25].position = { -0.5, -0.5, 0.5 };
	vertices[25].normal = { -1,0,0 };
	vertices[25].colour = { 1, 0, 0 };

	vertices[26].position = { -0.5, 0.5, -0.5 };
	vertices[26].normal = { -1,0,0 };
	vertices[26].colour = { 1, 0, 0 };

	vertices[27].position = { -0.5, -0.5, 0.5 };
	vertices[27].normal = { -1,0,0 };
	vertices[27].colour = { 1, 0, 0 };

	vertices[28].position = { -0.5, 0.5, -0.5 };
	vertices[28].normal = { -1,0,0 };
	vertices[28].colour = { 1, 0, 0 };

	vertices[29].position = { -0.5, 0.5, 0.5 };
	vertices[29].normal = { -1,0,0 };
	vertices[29].colour = { 1, 0, 0 };


	vertices[30].position = { 0.5, -0.5, -0.5 };
	vertices[30].normal = { 1,0,0 };
	vertices[30].colour = { 0, 1, 1 };

	vertices[31].position = { 0.5, -0.5, 0.5 };
	vertices[31].normal = { 1,0,0 };
	vertices[31].colour = { 0, 1, 1 };

	vertices[32].position = { 0.5, 0.5, -0.5 };
	vertices[32].normal = { 1,0,0 };
	vertices[32].colour = { 0, 1, 1 };

	vertices[33].position = { 0.5, -0.5, 0.5 };
	vertices[33].normal = { 1,0,0 };
	vertices[33].colour = { 0, 1, 1 };

	vertices[34].position = { 0.5, 0.5, -0.5 };
	vertices[34].normal = { 1,0,0 };
	vertices[34].colour = { 0, 1, 1 };

	vertices[35].position = { 0.5, 0.5, 0.5 };
	vertices[35].normal = { 1,0,0 };
	vertices[35].colour = { 0, 1, 1 };
	#pragma endregion

	Initialise(36, vertices);
}

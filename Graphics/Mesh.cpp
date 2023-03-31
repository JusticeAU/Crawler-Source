#include "Mesh.h"
#include "Object.h"

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
	glEnableVertexAttribArray(4); // Tangents
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

	// skinned mesh bone IDs and Weights
	glEnableVertexAttribArray(5); // boneID on mesh
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneID));
	glEnableVertexAttribArray(6); // weight of bone
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeight));


	// Bind indicies if there are any
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

void Mesh::CalculateTangents(Vertex* vertices, unsigned int vertexCount, const std::vector<unsigned int>& indices)
{
	glm::vec4* tan1 = new glm::vec4[vertexCount * 2];
	glm::vec4* tan2 = tan1 + vertexCount;
	memset(tan1, 0, vertexCount * sizeof(glm::vec4) * 2);
	unsigned int indexCount = (unsigned int)indices.size();
	for (unsigned int a = 0; a < indexCount; a += 3) {
		long i1 = indices[a];
		long i2 = indices[a + 1];
		long i3 = indices[a + 2];
		const glm::vec3& v1 = vertices[i1].position;
		const glm::vec3& v2 = vertices[i2].position;
		const glm::vec3& v3 = vertices[i3].position;
		const glm::vec2& w1 = vertices[i1].uv;
		const glm::vec2& w2 = vertices[i2].uv;
		const glm::vec2& w3 = vertices[i3].uv;
		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;
		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
		float r = 1.0F / (s1 * t2 - s2 * t1);
		glm::vec4 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r, 0);
		glm::vec4 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r, 0);
		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;
		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}
	for (unsigned int a = 0; a < vertexCount; a++) {
		const glm::vec3& n = glm::vec3(vertices[a].normal);
		const glm::vec3& t = glm::vec3(tan1[a]);
		// Gram-Schmidt orthogonalize
		vertices[a].tangent = glm::vec4(glm::normalize(t - n * glm::dot(n, t)), 0);
		// Calculate handedness (direction of bitangent)
		vertices[a].tangent.w = (glm::dot(glm::cross(glm::vec3(n), glm::vec3(t)), glm::vec3(tan2[a])) < 0.0F) ? 1.0F : -1.0F;
	}
	delete[] tan1;
}

#include "Model.h"
#include "Mesh.h"

void Model::DrawAllSubMeshes()
{
	for (int i = 0; i < meshes.size(); i++)
		DrawSubMesh(i);
}

void Model::DrawSubMesh(int index)
{
	Mesh* mesh = meshes[index];

	glBindVertexArray(mesh->vao);

	// check if we're using index buffers on this mesh by checking if indexbufferObject is valid (was it set up?)
	if (mesh->ibo != 0) // Draw with index buffering
		glDrawElements(GL_TRIANGLES, 3 * mesh->tris, GL_UNSIGNED_INT, 0);
	else // draw simply.
		glDrawArrays(GL_TRIANGLES, 0, 3 * mesh->tris);
}

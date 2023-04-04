#include "Model.h"
#include "Mesh.h"

void Model::Draw()
{
	// Shader should be bound by parent Object before this point.
	for (auto mesh : meshes)
	{
		// Draw triangle
		glBindVertexArray(mesh->vao);

		// check if we're using index buffers on this mesh by hecking if indexbufferObject is valid (was it set up?)
		if (mesh->ibo != 0) // Draw with index buffering
			glDrawElements(GL_TRIANGLES, 3 * mesh->tris, GL_UNSIGNED_INT, 0);
		else // draw simply.
			glDrawArrays(GL_TRIANGLES, 0, 3 * mesh->tris);
	}
}
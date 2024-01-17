#include "Model.h"
#include "Mesh.h"

void Model::DrawAllSubMeshes()
{
	for (int i = 0; i < meshes.size(); i++)
		DrawSubMesh(i);
}

void Model::DrawSubMesh(int index)
{
	meshes[index]->Bind();
	meshes[index]->Draw();
	Mesh::Unbind();
}

#include "MeshManager.h"
#include "Graphics.h"
#include "assimp/scene.h"
#include "assimp/cimport.h"
#include <filesystem>
#include "LogUtils.h"

using std::vector;
namespace fs = std::filesystem;

MeshManager::MeshManager()
{
    s_instance = this;
    CreateCube();
	CreateQuad();
	LoadAllFiles();
}

Mesh* MeshManager::GetMesh(string name)
{
    auto meshIt = s_instance->meshes.find(name);
    if (meshIt == s_instance->meshes.end())
        return nullptr;
    else
        return meshIt->second;
}

bool MeshManager::LoadMesh(string name)
{
    return false;
}

void MeshManager::DrawGUI()
{
	ImGui::Begin("Mesh Manager");
	ImGui::BeginDisabled();
	int meshCount = s_instance->meshes.size();
	ImGui::DragInt("Mesh Count", &meshCount);
	for (auto m : s_instance->meshes)
	{
		ImGui::Text(m.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}

void MeshManager::CreateCube()
{
    Mesh* cube = new Mesh();
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

	cube->Initialise(36, vertices);
	meshes.emplace("cube", cube);
}

void MeshManager::CreateQuad()
{
	Mesh* quad = new Mesh();
	// Using Index Buffer
	// define 4 vertices for 2 triangles
	Mesh::Vertex vertices[4];
	vertices[0].position = { -0.5f, 0, 0.5f };
	vertices[1].position = { 0.5f, 0, 0.5f };
	vertices[2].position = { -0.5f, 0, -0.5f };
	vertices[3].position = { 0.5f, 0, -0.5f };
	// define index buffer
	unsigned int indices[6] = { 0, 1, 2, 2, 1, 3 };

	// define normals
	vertices[0].normal = { 0, 1, 0 };
	vertices[1].normal = { 0, 1, 0 };
	vertices[2].normal = { 0, 1, 0 };
	vertices[3].normal = { 0, 1, 0 };

	// Define texCoords (UVs)
	vertices[0].uv = { 0.0f, 1.0f };
	vertices[1].uv = { 1.0f, 1.0f };
	vertices[2].uv = { 0.0f, 0.0f };
	vertices[3].uv = { 1.0f, 0.0f };

	quad->Initialise(4, vertices, 6, indices);
	meshes.emplace("quad", quad);
}

void MeshManager::LoadFromFile(const char* filename)
{
	// read all verticies from the model
	const aiScene* scene = aiImportFile(filename, 0);

	// Just use the first mesh for now.
	aiMesh* mesh = scene->mMeshes[0];

	// Extract indicies from first mesh
	int numFaces = mesh->mNumFaces;
	vector<unsigned int> indices;

	for (int i = 0; i < numFaces; i++)
	{
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);

		// generate a second tri for quads

		if (mesh->mFaces[i].mNumIndices == 4)
		{
			indices.push_back(mesh->mFaces[i].mIndices[0]);
			indices.push_back(mesh->mFaces[i].mIndices[3]);
			indices.push_back(mesh->mFaces[i].mIndices[2]);
		}
	}

	// Extract vertex data
	int numV = mesh->mNumVertices;
	Mesh::Vertex* vertices = new Mesh::Vertex[numV];
	for (int i = 0; i < numV; i++)
	{
		vertices[i].position = vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z);
		vertices[i].normal = vec3(
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z);
		if (mesh->mTextureCoords[0])
		{
			vertices[i].uv = vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y);
		}
		else
			vertices[i].uv = glm::vec2(0);
	}

	Mesh* loadedMesh = new Mesh();
	loadedMesh->Initialise(numV, vertices, indices.size(), indices.data());
	delete[] vertices;

	meshes.emplace(filename, loadedMesh);
}

void MeshManager::LoadAllFiles()
{
	LogUtils::Log("Loading models");
	for (auto d : fs::recursive_directory_iterator("models"))
	{
		if (d.path().extension() == ".obj" || d.path().extension() == ".fbx")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			LoadFromFile(d.path().generic_string().c_str());
		}
			
	}
}

MeshManager* MeshManager::s_instance = nullptr;
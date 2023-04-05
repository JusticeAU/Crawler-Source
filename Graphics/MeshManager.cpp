#include "MeshManager.h"
#include "Graphics.h"
#include "assimp/scene.h"
#include "LogUtils.h"
#include "Object.h"
#include "ai2glm.h"

using std::vector;

MeshManager::MeshManager()
{
	meshes.emplace("_null", nullptr);
    CreateCube();
	CreateQuad();
}

void MeshManager::Init()
{
	if (!s_instance) s_instance = new MeshManager();
	else LogUtils::Log("Tried to Init MeshManager when it was already initialised");
}

Mesh* MeshManager::GetMesh(string name)
{
    auto meshIt = s_instance->meshes.find(name);
    if (meshIt == s_instance->meshes.end())
        return nullptr;
    else
        return meshIt->second;
}

void MeshManager::DrawGUI()
{
	ImGui::Begin("Mesh Manager");
	ImGui::BeginDisabled();
	int meshCount = (int)s_instance->meshes.size();
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
	meshes.emplace("_cube", cube);
}

void MeshManager::CreateQuad()
{
	Mesh* quad = new Mesh();
	// Using Index Buffer
	// define 4 vertices for 2 triangles
	Mesh::Vertex vertices[4];
	vertices[0].position = { -0.5f, 0.0f, 0.5f };
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
	meshes.emplace("_quad", quad);
}

Mesh* MeshManager::LoadFromAiMesh(const aiMesh* mesh, Model::BoneStructure* boneStructure, const char* name)
{
	// Extract indices from first mesh
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

		if (mesh->HasTangentsAndBitangents())
		{
			vertices[i].tangent = vec4(
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z, 1);
		}
	}

	if (mesh->HasTangentsAndBitangents() == false)
	{
		Mesh::CalculateTangents(vertices, numV, indices);
	}

	// Begin creating mesh
	Mesh* loadedMesh = new Mesh();

	// Load bone data.
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		// find or allocate bone ID;
		string boneName = mesh->mBones[i]->mName.data;
		int boneIndex;
		auto bone = boneStructure->boneMapping.find(boneName);
		if (bone == boneStructure->boneMapping.end())
		{
			boneIndex = boneStructure->numBones;
			boneStructure->boneMapping.emplace(boneName, boneIndex);
			boneStructure->numBones++;
			boneStructure->boneInfo.resize(boneStructure->numBones);
			/*LogUtils::Log("Creating new Bone:");
			LogUtils::Log(boneName.c_str());
			LogUtils::Log(std::to_string(boneIndex).c_str());*/
		}
		else
		{
			boneIndex = bone->second;
		}

		// bone offset boi
		boneStructure->boneInfo[boneIndex].offset = mat4_cast(mesh->mBones[i]->mOffsetMatrix);

		// process all weights associated with the bone
		for (unsigned int boneWeightIndex = 0; boneWeightIndex < mesh->mBones[i]->mNumWeights; boneWeightIndex++)
		{
			int vertID = mesh->mBones[i]->mWeights[boneWeightIndex].mVertexId;
			float vertWeight = mesh->mBones[i]->mWeights[boneWeightIndex].mWeight;

			// find a slot on this vert to allocate this bone id and weight
			for (int j = 0; j < 4; j++)
			{
				if (vertices[vertID].boneID[j] == -1) // free slot
				{
					vertices[vertID].boneID[j] = boneIndex; // assign our bone index to it
					vertices[vertID].boneWeight[j] = vertWeight; // and weight
					//string log = "Placed bone ID" + std::to_string(i) + " in to slot " + std::to_string(j);
					//LogUtils::Log(log.c_str());
					break;
				}
				if (j > 3)
				{
					// shouldn't get here - if we did then there is a vert with more than 4 bones allocated to it. rip!
					LogUtils::Log("More than 4 bones affecting this vert!!!");
					continue;
				}
			}
		}
	}

	// Initialise mesh in OGL
	loadedMesh->Initialise(numV, vertices, (int)indices.size(), indices.data());
	delete[] vertices;
	s_instance->meshes.emplace(name, loadedMesh);
	return loadedMesh;
}

void MeshManager::CopyNodeHierarchy(const aiScene* scene, aiNode* node, Object* parent, Model::BoneStructure* boneStructure)
{
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		Object* object = new Object(0, node->mChildren[i]->mName.C_Str());
		parent->children.push_back(object);
		object->parent = parent;
		object->localTransform = mat4_cast(node->mChildren[i]->mTransformation);
		CopyNodeHierarchy(scene, node->mChildren[i], object, boneStructure);
	}
}

MeshManager* MeshManager::s_instance = nullptr;
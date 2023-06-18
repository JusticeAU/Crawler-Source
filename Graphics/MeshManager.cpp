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
	CreateCrawlCube();
	CreateQuad();
	CreateFullScreenQuad();
}

MeshManager::~MeshManager()
{
	for (auto mesh : meshes)
		delete mesh.second;
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
	ImGui::SetNextWindowPos({ 400, 20 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 880 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Meshes", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
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
	Mesh::Vertex vertices[24];

	#pragma region cube
	// Front
	vertices[0].position = { 1, 1, 1 };
	vertices[0].normal = { 0,0,1 };
	vertices[0].uv = { 1,1 };

	vertices[1].position = {-1, 1, 1 };
	vertices[1].normal = { 0,0,1 };
	vertices[1].uv = { 0,1 };

	vertices[2].position = {-1,-1, 1 };
	vertices[2].normal = { 0,0,1 };
	vertices[2].uv = { 0,0 };

	vertices[3].position = { 1,-1, 1 };
	vertices[3].normal = { 0,0,1 };
	vertices[3].uv = { 1,0 };

	// Right
	vertices[4].position = { 1, 1,-1 };
	vertices[4].normal = { 1, 0,0 };
	vertices[4].uv = { 1,1 };

	vertices[5].position = { 1, 1, 1 };
	vertices[5].normal = { 1, 0,0 };
	vertices[5].uv = { 0,1 };

	vertices[6].position = { 1,-1, 1 };
	vertices[6].normal = { 1, 0, 0 };
	vertices[6].uv = { 0,0 };

	vertices[7].position = { 1,-1,-1 };
	vertices[7].normal = { 1, 0,0 };
	vertices[7].uv = { 1,0 };

	// Back
	vertices[8].position = {-1, 1,-1 };
	vertices[8].normal = { 0,0,-1 };
	vertices[8].uv = { 1,1 };

	vertices[9].position = { 1, 1,-1 };
	vertices[9].normal = { 0,0,1 };
	vertices[9].uv = { 0,1 };

	vertices[10].position = { 1,-1,-1 };
	vertices[10].normal = { 0,0,1 };
	vertices[10].uv = { 0,0 };

	vertices[11].position = {-1,-1,-1 };
	vertices[11].normal = { 0,0,1 };
	vertices[11].uv = { 1,0 };

	// Left
	vertices[12].position = {-1, 1, 1 };
	vertices[12].normal = { -1,0,0 };
	vertices[12].uv = { 1,1 };

	vertices[13].position = {-1, 1,-1 };
	vertices[13].normal = { -1,0,0 };
	vertices[13].uv = { 0,1 };

	vertices[14].position = {-1,-1,-1 };
	vertices[14].normal = { -1,0,0 };
	vertices[14].uv = { 0,0 };

	vertices[15].position = {-1,-1, 1 };
	vertices[15].normal = { -1,0,0 };
	vertices[15].uv = { 1,0 };

	// Top
	vertices[16].position = { 1, 1,-1 };
	vertices[16].normal = { 0,1,0 };
	vertices[16].uv = { 1,1 };

	vertices[17].position = {-1, 1,-1 };
	vertices[17].normal = { 0,1,0 };
	vertices[17].uv = { 0,1 };

	vertices[18].position = {-1, 1, 1 };
	vertices[18].normal = { 0,1,0 };
	vertices[18].uv = { 0,0 };

	vertices[19].position = { 1, 1, 1 };
	vertices[19].normal = { 0,1,0 };
	vertices[19].uv = { 1,0 };

	// Bottom
	vertices[20].position = { 1,-1, 1 };
	vertices[20].normal = { 0,-1,0 };
	vertices[20].uv = { 1,1 };

	vertices[21].position = {-1,-1, 1 };
	vertices[21].normal = { 0,-1,0 };
	vertices[21].uv = { 0,1 };

	vertices[22].position = {-1,-1,-1 };
	vertices[22].normal = { 0,-1,0 };
	vertices[22].uv = { 0,0 };

	vertices[23].position = { 1,-1,-1 };
	vertices[23].normal = { 0,-1,0 };
	vertices[23].uv = { 1,0 };

	// Indicies
	unsigned int indicies[36] =
	{
		0, 1, 2, 0 ,2, 3, // front
		4, 5, 6, 4, 6, 7, // right
		8, 9,10, 8,10,11, // back
		12,13,14,12,14,15, // left
		16,17,18,16,18,19, // top
		20,21,22,20,22,23 // bottom
	};

	#pragma endregion

	cube->Initialise(24, vertices, 36, indicies);
	meshes.emplace("_cube", cube);
}

void MeshManager::CreateCrawlCube()
{
	Mesh* cube = new Mesh();
	Mesh::Vertex vertices[24];

#pragma region cube
	// Front
	vertices[0].position = { 2.5,	5,	2.5 };
	vertices[0].normal = { 0,0,1 };
	vertices[0].uv = { 1,1 };

	vertices[1].position = { -2.5,	5,	2.5 };
	vertices[1].normal = { 0,0,1 };
	vertices[1].uv = { 0,1 };

	vertices[2].position = { -2.5,	0,	2.5 };
	vertices[2].normal = { 0,0,1 };
	vertices[2].uv = { 0,0 };

	vertices[3].position = { 2.5,	0,	2.5 };
	vertices[3].normal = { 0,0,1 };
	vertices[3].uv = { 1,0 };

	// Right
	vertices[4].position = { 2.5, 5,	-2.5 };
	vertices[4].normal = { 1, 0,0 };
	vertices[4].uv = { 1,1 };

	vertices[5].position = { 2.5, 5, 2.5 };
	vertices[5].normal = { 1, 0,0 };
	vertices[5].uv = { 0,1 };

	vertices[6].position = { 2.5,0, 2.5 };
	vertices[6].normal = { 1, 0, 0 };
	vertices[6].uv = { 0,0 };

	vertices[7].position = { 2.5,0,-2.5 };
	vertices[7].normal = { 1, 0,0 };
	vertices[7].uv = { 1,0 };

	// Back
	vertices[8].position = { -2.5,	5,	-2.5 };
	vertices[8].normal = { 0,0,-1 };
	vertices[8].uv = { 1,1 };

	vertices[9].position = { 2.5,	5,	-2.5 };
	vertices[9].normal = { 0,0,1 };
	vertices[9].uv = { 0,1 };

	vertices[10].position = { 2.5,	0,	-2.5 };
	vertices[10].normal = { 0,0,1 };
	vertices[10].uv = { 0,0 };

	vertices[11].position = { -2.5,	0,	-2.5 };
	vertices[11].normal = { 0,0,1 };
	vertices[11].uv = { 1,0 };

	// Left
	vertices[12].position = { -2.5,	5,	2.5 };
	vertices[12].normal = { -1,0,0 };
	vertices[12].uv = { 1,1 };

	vertices[13].position = { -2.5,	5,	-2.5 };
	vertices[13].normal = { -1,0,0 };
	vertices[13].uv = { 0,1 };

	vertices[14].position = { -2.5,	0,	-2.5 };
	vertices[14].normal = { -1,0,0 };
	vertices[14].uv = { 0,0 };

	vertices[15].position = { -2.5,	0, 2.5 };
	vertices[15].normal = { -1,0,0 };
	vertices[15].uv = { 1,0 };

	// Top
	vertices[16].position = { 2.5, 5,-2.5 };
	vertices[16].normal = { 0,1,0 };
	vertices[16].uv = { 1,1 };

	vertices[17].position = { -2.5, 5,-2.5 };
	vertices[17].normal = { 0,1,0 };
	vertices[17].uv = { 0,1 };

	vertices[18].position = { -2.5, 5, 2.5 };
	vertices[18].normal = { 0,1,0 };
	vertices[18].uv = { 0,0 };

	vertices[19].position = { 2.5, 5, 2.5 };
	vertices[19].normal = { 0,1,0 };
	vertices[19].uv = { 1,0 };

	// Bottom
	vertices[20].position = { 2.5,	0,	2.5 };
	vertices[20].normal = { 0,-1,0 };
	vertices[20].uv = { 1,1 };

	vertices[21].position = { -2.5,	0,	2.5 };
	vertices[21].normal = { 0,-1,0 };
	vertices[21].uv = { 0,1 };

	vertices[22].position = { -2.5,	0,	-2.5 };
	vertices[22].normal = { 0,-1,0 };
	vertices[22].uv = { 0,0 };

	vertices[23].position = { 2.5,	0,	-2.5 };
	vertices[23].normal = { 0,-1,0 };
	vertices[23].uv = { 1,0 };

	// Indicies
	unsigned int indicies[36] =
	{
		0, 1, 2, 0 ,2, 3, // front
		4, 5, 6, 4, 6, 7, // right
		8, 9,10, 8,10,11, // back
		12,13,14,12,14,15, // left
		16,17,18,16,18,19, // top
		20,21,22,20,22,23 // bottom
	};

#pragma endregion

	cube->Initialise(24, vertices, 36, indicies);
	meshes.emplace("_crawlCube", cube);
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
	vertices[0].uv = { 0.0f, 0.0f };
	vertices[1].uv = { 1.0f, 0.0f };
	vertices[2].uv = { 0.0f, 1.0f };
	vertices[3].uv = { 1.0f, 1.0f };

	quad->Initialise(4, vertices, 6, indices);
	meshes.emplace("_quad", quad);
}

void MeshManager::CreateFullScreenQuad()
{
	Mesh* fsQuad = new Mesh();
	// Using Index Buffer
	// define 4 vertices for 2 triangles
	Mesh::Vertex vertices[4];
	vertices[0].position = {  1,  1, 0 };
	vertices[1].position = { -1,  1, 0 };
	vertices[2].position = { -1, -1, 0 };
	vertices[3].position = {  1, -1, 0 };
	// define index buffer
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };

	// define normals
	vertices[0].normal = { 0, 1, 0 };
	vertices[1].normal = { 0, 1, 0 };
	vertices[2].normal = { 0, 1, 0 };
	vertices[3].normal = { 0, 1, 0 };

	// Define texCoords (UVs)
	vertices[0].uv = { 1.0f, 1.0f };
	vertices[1].uv = { 0.0f, 1.0f };
	vertices[2].uv = { 0.0f, 0.0f };
	vertices[3].uv = { 1.0f, 0.0f };

	fsQuad->Initialise(4, vertices, 6, indices);
	meshes.emplace("_fsQuad", fsQuad);
	
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

	// Base implementation of rotation on import
	mat4 rotation = mat4(1);
	//rotation = glm::rotate(mat4(1), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Y up to Z up.
	for (int i = 0; i < numV; i++)
	{
		vec4 position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1 };
		position = rotation * position;
		vertices[i].position = (vec3)position;

		vec4 normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0 };
		normal = rotation * normal;
		vertices[i].normal = (vec3)normal;

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
		Mesh::CalculateTangents(vertices, numV, indices);

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
			boneStructure->boneOffsets.resize(boneStructure->numBones);
			/*LogUtils::Log("Creating new Bone:");
			LogUtils::Log(boneName.c_str());
			LogUtils::Log(std::to_string(boneIndex).c_str());*/
		}
		else
		{
			boneIndex = bone->second;
		}

		// Store the offset.
		boneStructure->boneOffsets[boneIndex] = mat4_cast(mesh->mBones[i]->mOffsetMatrix);
		//boneStructure->boneOffsets[boneIndex] = rotation * boneStructure->boneOffsets[boneIndex];

		// process all weights associated with the bone
		for (unsigned int boneWeightIndex = 0; boneWeightIndex < mesh->mBones[i]->mNumWeights; boneWeightIndex++)
		{
			int vertID = mesh->mBones[i]->mWeights[boneWeightIndex].mVertexId;
			float vertWeight = mesh->mBones[i]->mWeights[boneWeightIndex].mWeight;


			// find a slot on this vert to allocate this bone id and weight
			int minIndex = -1; // We're going to track the smallest one for if we need to drop one.
			float minWeight = FLT_MAX;

			bool storedBone = false;
			for (int j = 0; j < 4; j++)
			{
				if (vertices[vertID].boneID[j] == -1) // free slot
				{
					vertices[vertID].boneID[j] = boneIndex; // assign our bone index to it
					vertices[vertID].boneWeight[j] = vertWeight; // and weight
					storedBone = true;
					//string log = "Placed bone ID" + std::to_string(i) + " in to slot " + std::to_string(j);
					//LogUtils::Log(log.c_str());
					break;
				}

				if (minWeight > vertices[vertID].boneWeight[j])
				{
					minWeight = vertices[vertID].boneWeight[j];
					minIndex = j;
				}
			}

			if (!storedBone)
			{
				// shouldn't get here - we've configured Assimp to handle this on loading the asset file.
				LogUtils::Log("More than 4 bones affecting this vert!!!");
				LogUtils::Log("Dropping off least influenced bone");
				vertices[vertID].boneID[minIndex] = boneIndex;
				vertices[vertID].boneWeight[minIndex] = vertWeight;
				// Scale weights back to 1
				for (int j = 0; j < 4; j++)
					vertices[vertID].boneWeight[j] += minWeight * 0.25f;
			}
		}
	}

	// Ensure all bone weights per vertex total to 1.0f 
	if (mesh->mNumBones > 0)
	{
		bool triggered = false;
		for (int i = 0; i < numV; i++)
		{
			// Get bone count
			int weights = 0;
			float totalWeight = 0.0f;
			for (int j = 0; j < 4; j++)
			{
				if (vertices[i].boneID[j] == -1)
				{
					weights = j;
					break;
				}

				weights++;
				totalWeight += vertices[i].boneWeight[j];
			}
			const float weightAccuracyThreshold = 0.01f;
			if (glm::abs(1.0 - totalWeight) > weightAccuracyThreshold && !triggered)
			{
				triggered = true;
				LogUtils::Log("***** Detected large bone weight error ***** Correcting, but artist should fix this ******");
				LogUtils::Log(name);
			}

			// Divide each weight by total boneweight to scale em
			for (int j = 0; j < weights; j++)
			{
				vertices[i].boneWeight[j] /= totalWeight;
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
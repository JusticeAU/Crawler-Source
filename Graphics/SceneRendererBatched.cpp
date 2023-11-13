#include "SceneRendererBatched.h"
#include "ComponentAnimator.h"
#include "ComponentRenderer.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "Model.h"
#include "Object.h"
#include "Scene.h"
#include "SceneRenderer.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "TextureManager.h"
#include "UniformBuffer.h"


void RenderBatch::SetMatricies(glm::mat4 projectionView, glm::vec3 position)
{
	pv = projectionView;
	cameraPosition = position;
}

void RenderBatch::AddDraw(ComponentRenderer* renderer, int meshIndex)
{
	bool isSkinned = renderer->isAnimated;
	
	ShaderProgram* shader = nullptr;
	if (isSkinned) shader = renderer->submeshMaterials[meshIndex]->shaderSkinned;
	else shader = renderer->submeshMaterials[meshIndex]->shader;

	Material* material = renderer->submeshMaterials[meshIndex];
	Model* model = renderer->model;

	renderBatch.shaderBatches[shader].materialBatches[material].modelBatches[model].meshBatches[meshIndex].push_back(renderer);
}

void RenderBatch::DrawBatches()
{
	SceneRenderer::Statistics newStat;
	SceneRenderer::statistic = newStat;
	SceneRenderer::statistic.shaderBatches = renderBatch.shaderBatches.size();
	// Go for shaders
	for (auto& shaderBatch : renderBatch.shaderBatches)
	{
		ShaderProgram* shader = shaderBatch.first;
		shader->Bind();
		shader->SetVector3Uniform("cameraPosition", cameraPosition);
		shader->SetMatrixUniform("lightSpaceMatrix", Scene::GetLightSpaceMatrix());
		shader->SetFloatUniform("ambient", SceneRenderer::ambient);

		int numPointLights = Scene::GetNumPointLights();
		if (numPointLights > 0)
		{
			SceneRenderer::pointLightCubeMapDynamic[0]->BindTexture(11);
			shader->SetIntUniform("shadowMap0", 11);
		}
		if (numPointLights > 1)
		{
			SceneRenderer::pointLightCubeMapDynamic[1]->BindTexture(12);
			shader->SetIntUniform("shadowMap1", 12);
		}
		if (numPointLights > 2)
		{
			SceneRenderer::pointLightCubeMapDynamic[2]->BindTexture(13);
			shader->SetIntUniform("shadowMap2", 13);
		}
		if (numPointLights > 3)
		{
			SceneRenderer::pointLightCubeMapDynamic[3]->BindTexture(14);
			shader->SetIntUniform("shadowMap3", 14);
		}
		if (numPointLights > 4)
		{
			SceneRenderer::pointLightCubeMapDynamic[4]->BindTexture(15);
			shader->SetIntUniform("shadowMap4", 15);
		}
		if (numPointLights > 5)
		{
			SceneRenderer::pointLightCubeMapDynamic[5]->BindTexture(16);
			shader->SetIntUniform("shadowMap5", 16);
		}
		if (numPointLights > 6)
		{
			SceneRenderer::pointLightCubeMapDynamic[6]->BindTexture(17);
			shader->SetIntUniform("shadowMap6", 17);
		}
		if (numPointLights > 7)
		{
			SceneRenderer::pointLightCubeMapDynamic[7]->BindTexture(18);
			shader->SetIntUniform("shadowMap7", 18);
		}
		if (numPointLights > 8)
		{
			SceneRenderer::pointLightCubeMapDynamic[8]->BindTexture(19);
			shader->SetIntUniform("shadowMap8", 19);
		}
		if (numPointLights > 9)
		{
			SceneRenderer::pointLightCubeMapDynamic[9]->BindTexture(20);
			shader->SetIntUniform("shadowMap9", 20);
		}
		if (numPointLights > 10)
		{
			SceneRenderer::pointLightCubeMapDynamic[10]->BindTexture(21);
			shader->SetIntUniform("shadowMap10", 21);
		}
		if (numPointLights > 11)
		{
			SceneRenderer::pointLightCubeMapDynamic[11]->BindTexture(22);
			shader->SetIntUniform("shadowMap11", 22);
		}
		if (numPointLights > 12)
		{
			SceneRenderer::pointLightCubeMapDynamic[12]->BindTexture(23);
			shader->SetIntUniform("shadowMap12", 23);
		}
		if (numPointLights > 13)
		{
			SceneRenderer::pointLightCubeMapDynamic[13]->BindTexture(24);
			shader->SetIntUniform("shadowMap13", 24);
		}
		if (numPointLights > 14)
		{
			SceneRenderer::pointLightCubeMapDynamic[14]->BindTexture(25);
			shader->SetIntUniform("shadowMap14", 25);
		}
		if (numPointLights > 15)
		{
			SceneRenderer::pointLightCubeMapDynamic[15]->BindTexture(26);
			shader->SetIntUniform("shadowMap15", 26);
		}
		if (numPointLights > 16)
		{
			SceneRenderer::pointLightCubeMapDynamic[16]->BindTexture(27);
			shader->SetIntUniform("shadowMap16", 27);
		}
		if (numPointLights > 17)
		{
			SceneRenderer::pointLightCubeMapDynamic[17]->BindTexture(28);
			shader->SetIntUniform("shadowMap17", 28);
		}
		if (numPointLights > 18)
		{
			SceneRenderer::pointLightCubeMapDynamic[18]->BindTexture(29);
			shader->SetIntUniform("shadowMap18", 29);
		}
		if (numPointLights > 19)
		{
			SceneRenderer::pointLightCubeMapDynamic[19]->BindTexture(30);
			shader->SetIntUniform("shadowMap19", 30);
		}


		// Go for Materials
		SceneRenderer::statistic.materialBatches += shaderBatch.second.materialBatches.size();
		for (auto& materialBatch : shaderBatch.second.materialBatches)
		{
			Material* material = materialBatch.first;
			if (material->isPBR)
			{
				if (material->blendMode == Material::BlendMode::AlphaCutoff)
					shader->SetBoolUniform("useAlphaCutoff", true);
				else
					shader->SetBoolUniform("useAlphaCutoff", false);

				if (material->albedoMap)
				{
					material->albedoMap->Bind(4);
					shader->SetIntUniform("albedoMap", 4);
				}
				if (material->normalMap)
				{
					material->normalMap->Bind(5);
					shader->SetIntUniform("normalMap", 5);
				}
				if (material->metallicMap)
				{
					material->metallicMap->Bind(6);
					shader->SetIntUniform("metallicMap", 6);
				}
				else
				{
					TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(6);
					shader->SetIntUniform("metallicMap", 6);
				}

				if (material->roughnessMap)
				{
					material->roughnessMap->Bind(7);
					shader->SetIntUniform("roughnessMap", 7);
				}
				else
				{
					TextureManager::GetTexture("engine/texture/grey1x1.tga")->Bind(7);
					shader->SetIntUniform("roughnessMap", 8);
				}

				if (material->aoMap)
				{
					material->aoMap->Bind(8);
					shader->SetIntUniform("aoMap", 8);
				}
				else
				{
					TextureManager::GetTexture("engine/texture/white1x1.tga")->Bind(8);
					shader->SetIntUniform("aoMap", 8);
				}

				if (material->emissiveMap)
				{
					material->emissiveMap->Bind(9);
					shader->SetIntUniform("emissiveMap", 9);
				}
				else
				{
					TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(9);
					shader->SetIntUniform("emissiveMap", 9);
				}
			}
			else
			{
				shader->SetVector3Uniform("Ka", material->Ka);
				shader->SetVector3Uniform("Kd", material->Kd);
				shader->SetVector3Uniform("Ks", material->Ks);
				shader->SetFloatUniform("specularPower", material->specularPower);

				if (material->mapKd)
				{
					material->mapKd->Bind(1);
					shader->SetIntUniform("diffuseTex", 1);
				}
				if (material->mapKs)
				{
					material->mapKs->Bind(2);
					shader->SetIntUniform("specularTex", 2);
				}
				if (material->mapBump)
				{
					material->mapBump->Bind(3);
					shader->SetIntUniform("normalTex", 3);
				}
			}

			// Go for Models
			for (auto& modelBatch : materialBatch.second.modelBatches)
			{

				// Bind shadow maps based on the cursed way im doing it right now.
				// go for meshes
				MeshBatch batch = modelBatch.second;
				for (auto& meshBatch : batch.meshBatches)
				{
					// Draw Meshes
					for (auto& renderer : meshBatch.second)
					{
						Object* object = renderer->GetComponentParentObject();
						
						// Combine the matricies
						glm::mat4 modelMat = object->transform * renderer->model->modelTransform;
						glm::mat4 pvm = pv * modelMat;
						shader->SetMatrixUniform("pvmMatrix", pvm);
						shader->SetMatrixUniform("mMatrix", modelMat);
						
						if (renderer->isAnimated)
						{
							shader->SetUniformBlockIndex("boneTransformBuffer", 0);
							renderer->animator->boneTransfomBuffer->Bind(0);
							renderer->animator->boneTransfomBuffer->SendData(renderer->animator->boneTransforms);
						}

						// Set up shadow map the way we currently do them and this needs to change
						renderer->closestPointLightUBO->Bind(3);
						shader->SetUniformBlockIndex("pointLightIndicesBuffer", 3);

						int numLights = Scene::GetNumPointLights();
						shader->SetIntUniform("numLights", numLights);
						if (renderer->receivesShadows)
						{
							shader->SetFloatUniform("shadowBias", renderer->shadowBias);
						}

						shader->SetFloatUniform("emissiveScale", renderer->emissiveScale);

						// Draw the mesh
						modelBatch.first->DrawSubMesh(meshBatch.first);
						SceneRenderer::statistic.drawCalls++;
						SceneRenderer::statistic.tris += modelBatch.first->meshes[meshBatch.first]->tris;
					}
				}
			}
		}
	}
}

void RenderBatch::ClearBatches()
{
	renderBatch.shaderBatches.clear();
	//for (auto& material : renderBatch.shaderBatches)
	//{
	//	for (auto& model : material.second.materialBatches)
	//	{
	//		for (auto& mesh : model.second.modelBatches)
	//		{
	//			mesh.second.meshBatches.clear();
	//		}
	//	}
	//}
}

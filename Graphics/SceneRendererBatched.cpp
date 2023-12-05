#include "SceneRendererBatched.h"
#include "ComponentAnimator.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "FrameBuffer.h"
#include "Material.h"
#include "Model.h"
#include "Object.h"
#include "Scene.h"
#include "SceneRenderer.h"
#include "ShaderProgram.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "TextureManager.h"
#include "UniformBuffer.h"


void RenderBatch::SetCameraMatricies(ComponentCamera* camera)
{
	viewMatrix = camera->GetViewMatrix();
	projectionViewMatrix = camera->GetViewProjectionMatrix();;
	cameraPosition = camera->GetWorldSpacePosition();
}

void RenderBatch::AddDraw(ComponentRenderer* renderer, int meshIndex)
{
	ShaderProgram* shader = nullptr;
	bool isSkinned = renderer->isAnimated;
	switch (renderPass)
	{
	case RenderPass::gBuffer:
	{	
		shader = ShaderManager::GetShaderProgram("engine/shader/SSAOGeometryPass");
		break;
	}
	case RenderPass::Opaque:
	{
		if (isSkinned) shader = renderer->submeshMaterials[meshIndex]->shaderSkinned;
		else shader = renderer->submeshMaterials[meshIndex]->shader;
		break;
	}
	}

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

		TextureManager::GetTexture("crawler/texture/perlin_noise.tga")->Bind(10);
		shader->SetIntUniform("perlinNoise", 10);
		
		switch (renderPass)
		{
		case RenderPass::gBuffer:
		{
			shader->SetMatrixUniform("view", viewMatrix);
			break;
		}
		case RenderPass::Opaque:
		{
			shader->SetVector3Uniform("cameraPosition", cameraPosition);
			shader->SetMatrixUniform("lightSpaceMatrix", Scene::GetLightSpaceMatrix());
			shader->SetFloatUniform("ambient", SceneRenderer::ambient);
			int numPointLights = Scene::GetNumPointLights();
			shader->SetIntUniform("shadowMapArray", 31);
			SceneRenderer::pointlightCubeMapArrayDynamic->BindTexture(31);
			break;
		}
		}


		// Go for Materials
		SceneRenderer::statistic.materialBatches += shaderBatch.second.materialBatches.size();
		for (auto& materialBatch : shaderBatch.second.materialBatches)
		{

			Material* material = materialBatch.first;
			if (material->backFaceCulling)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);



			switch (renderPass)
			{
			case RenderPass::gBuffer:
			{
				if (material->blendMode == Material::BlendMode::AlphaCutoff)
				{
					shader->SetBoolUniform("useAlphaCutoff", true);
					if (material->albedoMap) material->albedoMap->Bind(4);
					shader->SetIntUniform("albedoMap", 4);
				}
				else
					shader->SetBoolUniform("useAlphaCutoff", false);
				break;
			}
			case RenderPass::Opaque:
			{
				if (material->isPBR)
				{
					shader->SetFloatUniform("tiling", material->tiling);

					if (material->albedoMap) material->albedoMap->Bind(4);
					shader->SetIntUniform("albedoMap", 4);

					if (material->normalMap) material->normalMap->Bind(5);
					else TextureManager::GetTexture("engine/texture/normal1x1.tga")->Bind(5);
					shader->SetIntUniform("normalMap", 5);

					if (material->metallicMap) material->metallicMap->Bind(6);
					else TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(6);
					shader->SetIntUniform("metallicMap", 6);

					if (material->roughnessMap) material->roughnessMap->Bind(7);
					else TextureManager::GetTexture("engine/texture/grey1x1.tga")->Bind(7);
					shader->SetIntUniform("roughnessMap", 7);

					if (material->aoMap) material->aoMap->Bind(8);
					else TextureManager::GetTexture("engine/texture/white1x1.tga")->Bind(8);
					shader->SetIntUniform("aoMap", 8);

					if (material->emissiveMap) material->emissiveMap->Bind(9);
					else TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(9);
					shader->SetIntUniform("emissiveMap", 9);

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
				break;
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
						glm::mat4 pvm = projectionViewMatrix * modelMat;
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
						shader->SetFloatUniform("dissolveThreshold", renderer->dissolveThreshold);
						shader->SetFloatUniform("dissolveEdge", renderer->dissolveEdge);


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

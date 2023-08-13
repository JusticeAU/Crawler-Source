#include "ComponentSkinnedRenderer.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "ComponentAnimator.h"
#include "ComponentAnimationBlender.h"
#include "UniformBuffer.h"
#include "Model.h"

ComponentSkinnedRenderer::ComponentSkinnedRenderer(Object* parent) : ComponentRenderer(parent)
{
	componentName = "Skinned Renderer";
	componentType = Component_SkinnedRenderer;
}

ComponentSkinnedRenderer::ComponentSkinnedRenderer(Object* parent, ordered_json j) : ComponentRenderer(parent, j)
{
	componentName = "Skinned Renderer";
	componentType = Component_SkinnedRenderer;
}

void ComponentSkinnedRenderer::Draw(mat4 pv, vec3 position, DrawMode mode)
{
	if (model != nullptr && materialArray[0] != nullptr) // At minimum we need a model and a shader to draw something.
	{
		switch (mode)
		{
			case DrawMode::Standard:
			{
				for (int i = 0; i < model->GetMeshCount(); i++)
				{
					if (materialArray[i] != nullptr)
						material = materialArray[i];
					else
						continue;

					if (material->shader == nullptr)
						continue;

					BindShader();
					ApplyMaterials();
					BindMatricies(pv, position);
					SetUniforms();
					BindBoneTransform();
					model->DrawSubMesh(i);
				}
				break;
			}
			case DrawMode::ObjectPicking:
			{
				ShaderProgram* shader = ShaderManager::GetShaderProgram("engine/shader/skinnedPicking");
				shader->Bind();
				shader->SetUIntUniform("objectID", componentParent->id);
				shader->SetUniformBlockIndex("boneTransformBuffer", 0);
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform);
				shader->SetVector3Uniform("cameraPosition", position);
				BindBoneTransform();
				DrawModel();
				break;
			}
			case DrawMode::ShadowMapping:
			{
				ShaderProgram* shader = ShaderManager::GetShaderProgram("engine/shader/simpleDepthShaderSkinned");
				shader->Bind();
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform);
				shader->SetVector3Uniform("cameraPosition", position);
				BindBoneTransform();
				DrawModel();
				break;
			}
		}
	}
}

void ComponentSkinnedRenderer::OnParentChange()
{
	animator = nullptr;
	animationBlender = nullptr;

	ComponentRenderer::OnParentChange();

	Component* component = componentParent->GetComponent(Component_Animator);
	if (component)
	{
		animator = static_cast<ComponentAnimator*>(component);
	}
	else
	{
		component = componentParent->GetComponent(Component_AnimationBlender);
		animationBlender = static_cast<ComponentAnimationBlender*>(component);
	}
}

void ComponentSkinnedRenderer::BindBoneTransform()
{
	if (material == nullptr || material->shader == nullptr)
		return;

	// skinned mesh rendering
	//shader->SetIntUniform("selectedBone", animator->selectedBone); // dev testing really, used by boneWeights shader
	if (animator != nullptr && animator->boneTransfomBuffer != nullptr)
	{
		// get the shader uniform block index and set binding point - we'll just hardcode 0 for this.
		material->shader->SetUniformBlockIndex("boneTransformBuffer", 0);
		// TODO - this could be done in shader initialisation if it detected that that shader had this uniform buffer
		// It only needs to be done once per shader too, assuming im not reusing the index anywhere else (which im not)

		// Now link the UniformBufferObject to the bind point in the GL context.
		animator->boneTransfomBuffer->Bind(0);

		animator->boneTransfomBuffer->SendData(animator->boneTransforms);

		// Technically, all animated models can share the same boneTransformBuffer and just keep uploading their data in to it every frame.
		// A dedicated animation system could provide a single global buffer for the shader for this.
	}

	if (animationBlender != nullptr && animationBlender->boneTransfomBuffer != nullptr)
	{
		material->shader->SetUniformBlockIndex("boneTransformBuffer", 0);
		animationBlender->boneTransfomBuffer->Bind(0);
		animationBlender->boneTransfomBuffer->SendData(animationBlender->boneTransforms);
	}
}

Component* ComponentSkinnedRenderer::Clone(Object* parent)
{
	ComponentSkinnedRenderer* copy = new ComponentSkinnedRenderer(parent);
	copy->castsShadows = castsShadows;
	copy->material = material;
	copy->model = model;
	copy->receivesShadows = receivesShadows;
	copy->shadowBias = shadowBias;
	return copy;
}

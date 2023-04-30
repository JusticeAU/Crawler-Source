#include "ComponentSkinnedRenderer.h"
#include "ShaderManager.h"
#include "ComponentAnimator.h"
#include "ComponentAnimationBlender.h"
#include "UniformBuffer.h"

ComponentSkinnedRenderer::ComponentSkinnedRenderer(Object* parent) : ComponentRenderer(parent)
{
	componentName = "Skinned Renderer";
	componentType = Component_SkinnedRenderer;
}

ComponentSkinnedRenderer::ComponentSkinnedRenderer(Object* parent, std::istream& istream) : ComponentRenderer(parent, istream)
{
	componentName = "Skinned Renderer";
	componentType = Component_SkinnedRenderer;
}

void ComponentSkinnedRenderer::Draw(mat4 pv, vec3 position, DrawMode mode)
{
	if (model != nullptr && shader != nullptr) // At minimum we need a model and a shader to draw something.
	{
		switch (mode)
		{
			case DrawMode::Standard:
			{
				BindShader();
				BindMatricies(pv, position);
				break;
			}
			case DrawMode::ObjectPicking:
			{
				ShaderProgram* shad = ShaderManager::GetShaderProgram("shaders/skinnedPicking");
				shad->Bind();
				shad->SetUIntUniform("objectID", componentParent->id);
				shad->SetUniformBlockIndex("boneTransformBuffer", 0);
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shad->SetMatrixUniform("pvmMatrix", pvm);
				shad->SetMatrixUniform("mMatrix", componentParent->transform);
				shad->SetVectorUniform("cameraPosition", position);
				break;
			}
			case DrawMode::ShadowMapping:
			{
				ShaderProgram* shad = ShaderManager::GetShaderProgram("shaders/simpleDepthShaderSkinned");
				shad->Bind();
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shad->SetMatrixUniform("pvmMatrix", pvm);
				shad->SetMatrixUniform("mMatrix", componentParent->transform);
				shad->SetVectorUniform("cameraPosition", position);
				// shadow mapping shader config here
				break;
			}
		}
		
		SetUniforms();
		ApplyTexture();
		ApplyMaterials();
		
		BindBoneTransform(); // This is one is added on top of ComponentRenderer.
		DrawModel();
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
	// skinned mesh rendering
	//shader->SetIntUniform("selectedBone", animator->selectedBone); // dev testing really, used by boneWeights shader
	if (animator != nullptr && animator->boneTransfomBuffer != nullptr)
	{
		// get the shader uniform block index and set binding point - we'll just hardcode 0 for this.
		shader->SetUniformBlockIndex("boneTransformBuffer", 0);
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
		shader->SetUniformBlockIndex("boneTransformBuffer", 0);
		animationBlender->boneTransfomBuffer->Bind(0);
		animationBlender->boneTransfomBuffer->SendData(animationBlender->boneTransforms);
	}
}

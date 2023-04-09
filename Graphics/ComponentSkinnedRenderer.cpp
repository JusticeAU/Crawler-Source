#include "ComponentSkinnedRenderer.h"
#include "ShaderProgram.h"
#include "ComponentAnimator.h"
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

void ComponentSkinnedRenderer::Draw()
{
	if (model != nullptr && shader != nullptr) // At minimum we need a model and a shader to draw something.
	{
		BindShader();
		BindMatricies();
		SetUniforms();
		ApplyTexture();
		ApplyMaterials();
		if(animator)
			BindBoneTransform(); // This is one is added on top of ComponentRenderer.
		DrawModel();
	}
}

void ComponentSkinnedRenderer::OnParentChange()
{
	ComponentRenderer::OnParentChange();

	Component* component = componentParent->GetComponent(Component_Animator);
	if (component)
	{
		animator = static_cast<ComponentAnimator*>(component);
	}
}

void ComponentSkinnedRenderer::BindBoneTransform()
{
	// skinned mesh rendering
	shader->SetIntUniform("selectedBone", animator->selectedBone); // dev testing really, used by boneWeights shader
	if (animator->boneTransfomBuffer != nullptr)
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
}

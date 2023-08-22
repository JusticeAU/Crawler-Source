#include "SceneRenderer.h"
#include "FrameBuffer.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "Window.h"

#include "ComponentCamera.h"
#include "PostProcess.h"

#include "MathUtils.h"

#include <random>

// Stuff to move away
#include "SceneEditorCamera.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "ModelManager.h"
#include "MaterialManager.h"


bool SceneRenderer::msaaEnabled = true;
bool SceneRenderer::ssaoEnabled = true;
ComponentCamera* SceneRenderer::frustumCullingCamera = nullptr;
float SceneRenderer::frustumCullingForgiveness = 5.0f;

SceneRenderer::SceneRenderer()
{
	// Initialise the Render Buffers
#pragma region Render Buffer creation
	// 1 to render the scene to, a second to blit to if we are multisampling, and then a 3rd for the final post processing effects to land to.
	frameBufferRaw = new FrameBuffer(FrameBuffer::Type::CameraTargetMultiSample);
	frameBufferRaw->MakePrimaryTarget();
	string FbName = "Scene Renderer_FrameBuffer";
	TextureManager::s_instance->AddFrameBuffer(FbName.c_str(), frameBufferRaw); // add the texture to the manager so we can bind it to meshes and stuff.

	frameBufferBlit = new FrameBuffer(FrameBuffer::Type::CameraTargetSingleSample);
	string BlitName = FbName + "_Blit";
	TextureManager::s_instance->AddFrameBuffer(BlitName.c_str(), frameBufferBlit); // add the texture to the manager so we can bind it to meshes and stuff.

	// The final frame buffer after the post process of the camera has been performed.
	frameBufferProcessed = new FrameBuffer(FrameBuffer::Type::PostProcess);
	//cameras.push_back(frameBufferProcessed);
	string processedFBName = FbName + "_Processed";
	TextureManager::s_instance->AddFrameBuffer(processedFBName.c_str(), frameBufferProcessed);
#pragma endregion

	// Intialise SSAO Configuration
#pragma region SSAO Configuration
	ssaoGBuffer = new FrameBuffer(FrameBuffer::Type::SSAOgBuffer);
	TextureManager::s_instance->AddFrameBuffer("gBuffer", ssaoGBuffer);
	ssaoFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
	TextureManager::s_instance->AddFrameBuffer("SSAO", ssaoFBO);
	ssaoBlurFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
	TextureManager::s_instance->AddFrameBuffer("SSAOBlur", ssaoBlurFBO);
	// Generate the kernel - https://learnopengl.com/Advanced-Lighting/SSAO
	// Both from <random>
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	unsigned int kernalSize = 64;
	ssaoKernel.reserve(kernalSize);
	for (unsigned int i = 0; i < kernalSize; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / (float)kernalSize;
		scale = MathUtils::Lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
		//LogUtils::Log(to_string(sample.x) + " " + to_string(sample.y) + " " + to_string(sample.z));
	}

	// Generate SSAO Noise
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}
	ssaoNoiseTexture = new Texture();
	ssaoNoiseTexture->CreateSSAONoiseTexture(ssaoNoise.data());
	TextureManager::AddTexture(ssaoNoiseTexture);
#pragma endregion

	// Initialise Shadow Mapping
#pragma region Shadow Mapping
	shadowMap = new FrameBuffer(FrameBuffer::Type::ShadowMap);
	shadowMapDevOutput = new FrameBuffer(FrameBuffer::Type::PostProcess);
	depthMapOutputShader = ShaderManager::GetShaderProgram("engine/shader/zzShadowMapDev");
#pragma endregion

	// Initialise Object Picking
#pragma region Object Picking
	objectPickBuffer = new FrameBuffer(FrameBuffer::Type::ObjectPicker);
	TextureManager::s_instance->AddFrameBuffer("Objecting Picking Buffer", objectPickBuffer);
#pragma endregion
	// Initialise the hack Gizmo rendering
#pragma region Gizmo Rendering
	// Create our Light Gizmo for rendering - this will move to a ComponentLight and be handled there.
	lightGizmo = new Object(-1, "Light Gizmo");
	ComponentModel* lightGizmoModelComponent = new ComponentModel(lightGizmo);
	lightGizmoModelComponent->model = ModelManager::GetModel("engine/model/Gizmos/bulb.fbx");
	lightGizmo->components.push_back(lightGizmoModelComponent);
	ComponentRenderer* lightGizmoRenderer = new ComponentRenderer(lightGizmo);
	gizmoShader = ShaderManager::GetShaderProgram("engine/shader/gizmoShader");
	lightGizmoRenderer->model = lightGizmoModelComponent->model;
	lightGizmoRenderer->materialArray.resize(1);
	lightGizmoRenderer->materialArray[0] = MaterialManager::GetMaterial("engine/model/materials/Gizmos.material");
	lightGizmo->components.push_back(lightGizmoRenderer);
#pragma endregion
}

void SceneRenderer::DrawGUI()
{
	// Graphics Options - Abstract this as these options develop.
	ImGui::SetNextWindowSize({ 315, 450 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos({ 1200, 300 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Graphics");
	ImGui::BeginDisabled();
	
	float averageTotalRenderTime = 0.0f;
	for (int i = 0; i < 100; i++)
		averageTotalRenderTime += renderTotalSamples[i];
	averageTotalRenderTime /= 100;
	averageTotalRenderTime *= 1000;
	ImGui::InputFloat("Render MS Average", &averageTotalRenderTime, 0, 0, "%0.6f");
	ImGui::PlotLines("", renderTotalSamples, 100, 0, "0 to 0.1s", 0, 0.1, { 300,100 });

	ImGui::EndDisabled();
	if (ImGui::Checkbox("VSync", &vsyncEnabled))
	{
		if (vsyncEnabled) glfwSwapInterval(1);
		else glfwSwapInterval(0);
	}

	if (ImGui::Checkbox("Frustum Culling", &frustumCullingEnabled))
	{
		if (!frustumCullingEnabled) frustumCullingCamera = nullptr;
		else frustumCullingCamera = Scene::GetCameraByIndex(frustumCullingCameraIndex);
	}

	if (frustumCullingEnabled)
	{
		ImGui::Indent();
		if (ImGui::InputInt("Camera", &frustumCullingCameraIndex, 1))
			SetCullingCamera(frustumCullingCameraIndex);

		if (ImGui::DragFloat("Forgiveness", &frustumCullingForgiveness, 0.1f, -10, 10, "%0.1f"));
		ImGui::Unindent();
	}

	if (ImGui::Checkbox("MSAA", &msaaEnabled))
		TextureManager::RefreshFrameBuffers();
	
	if (msaaEnabled)
	{
		ImGui::Indent();
		int newMSAASample = msaaSamples;
		if (ImGui::InputInt("Samples", &newMSAASample))
		{
			if (newMSAASample > msaaSamples)
				msaaSamples *= 2;
			else
				msaaSamples /= 2;

			msaaSamples = glm::clamp(msaaSamples, 2, 16);
			FrameBuffer::SetMSAASampleLevels(msaaSamples);
			TextureManager::RefreshFrameBuffers();
		}
		ImGui::Unindent();
	}

	if (ImGui::Checkbox("SSAO", &ssaoEnabled))
	{
		if (!ssaoEnabled)
		{
			for (int i = 0; i < Scene::s_editorCamera->camera->m_postProcessStack.size(); i++)
			{
				if (Scene::s_editorCamera->camera->m_postProcessStack[i]->GetShaderName() == "engine/shader/postProcess/SSAO")
				{
					delete Scene::s_editorCamera->camera->m_postProcessStack[i];
					Scene::s_editorCamera->camera->m_postProcessStack.erase(Scene::s_editorCamera->camera->m_postProcessStack.begin() + i);
					break;
				}
			}

			for (auto& c : Scene::s_instance->componentCameras)
			{
				for (int i = 0; i < c->m_postProcessStack.size(); i++)
				{
					if (c->m_postProcessStack[i]->GetShaderName() == "engine/shader/postProcess/SSAO")
					{
						delete c->m_postProcessStack[i];
						c->m_postProcessStack.erase(c->m_postProcessStack.begin() + i);
						break;
					}
				}
			}
		}
		else
		{
			// Add to Scene Camera
			string ppName = "engine/shader/postProcess/SSAO";
			PostProcess* pp = new PostProcess(Scene::s_editorCamera->camera->GetComponentParentObject()->objectName + "_PP_" + ppName);
			pp->SetShader(ShaderManager::GetShaderProgram(ppName));
			pp->SetShaderName(ppName);
			Scene::s_editorCamera->camera->m_postProcessStack.push_back(pp);

			// Add to component Camaras in scene
			for (auto& c : Scene::s_instance->componentCameras)
			{
				ppName = "engine/shader/postProcess/SSAO";
				pp = new PostProcess(c->GetComponentParentObject()->objectName + "_PP_" + ppName);
				pp->SetShader(ShaderManager::GetShaderProgram(ppName));
				pp->SetShaderName(ppName);
				c->m_postProcessStack.push_back(pp);
			}
		}
	}
	if (ssaoEnabled)
	{
		ImGui::Indent();
		ImGui::PushID("Radius");
		ImGui::Text("Radius");
		ImGui::DragFloat("", &ssaoRadius, 0.01);
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
			ssaoRadius = 0.5f;
		ImGui::PopID();
		ImGui::PushID("Bias");
		ImGui::Text("Bias");
		ImGui::DragFloat("", &ssaoBias, 0.01);
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
			ssaoBias = 0.025f;
		ImGui::PopID();
		ImGui::Text("");
		ImGui::Unindent();
	}

	if (ImGui::Button("Reload Shaders"))
		ShaderManager::RecompileAllShaderPrograms();

	ImGui::End();
}

void SceneRenderer::DrawShadowMappingGUI()
{
	ImGui::Begin("Shadow Map Dev");
	ImGui::PushID(6969);
	ImGui::DragFloat("Ortho Near", &orthoNear);
	ImGui::DragFloat("Ortho Far", &orthoFar);
	ImGui::DragFloat("Ortho Left", &orthoLeft);
	ImGui::DragFloat("Ortho Right", &orthoRight);
	ImGui::DragFloat("Ortho Bottom", &orthoBottom);
	ImGui::DragFloat("Ortho Top", &orthoTop);
	ImGui::Image((ImTextureID)(shadowMapDevOutput->GetTexture()->texID), { 512,512 }, { 0,1 }, { 1,0 });

	ImGui::PopID();
	ImGui::End();
}

void SceneRenderer::RenderScene(Scene* scene, ComponentCamera* c)
{
	// Shadow map dev stuff
	// Bind recently rendered shadowmap.
	//shadowMap->BindTexture(20);
	//shadowMapDevOutput->BindTarget();
	//PostProcess::PassThrough(depthMapOutputShader);

	//shadowMap->BindTexture(5);
	// for each camera in each object, draw to that cameras frame buffer

	// Update the camera VPM and also the culling frustum if being used.
	c->UpdateViewProjectionMatrix();
	if (frustumCullingCamera != nullptr)
	{
		if(c != frustumCullingCamera) frustumCullingCamera->UpdateViewProjectionMatrix();

		frustumCullingCamera->UpdateFrustum();
	}
	vec3 cameraPosition = c->GetWorldSpacePosition();

	if (ssaoEnabled)
	{
		// Build G Buffer for SSAO with a Geomerty Pass.
		ssaoGBuffer->BindTarget();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ShaderProgram* ssaoGeoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOGeometryPass");
		ssaoGeoShader->Bind();
		ssaoGeoShader->SetMatrixUniform("view", c->GetViewMatrix());
		ssaoGeoShader->SetMatrixUniform("projection", c->GetProjectionMatrix());

		// bind the shader for it
		glDisable(GL_BLEND);
		for (auto& o : scene->objects)
			o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::SSAOgBuffer);
		glEnable(GL_BLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Do the SSAO pass
		ssaoFBO->BindTarget();
		glClear(GL_COLOR_BUFFER_BIT);

		ShaderProgram* ssaoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOTermPass");
		ssaoShader->Bind();
		ssaoShader->SetFloatUniform("radius", ssaoRadius);
		ssaoShader->SetFloatUniform("bias", ssaoBias);
		glm::vec2 screenSize = Window::GetViewPortSize();
		ssaoShader->SetVector2Uniform("screenSize", screenSize);
		
		glDisable(GL_BLEND);
		ssaoShader->SetIntUniform("gPosition", 0);
		ssaoShader->SetIntUniform("gNormal", 1);
		ssaoShader->SetIntUniform("texNoise", 2);

		ssaoShader->SetFloat3ArrayUniform("samples", 64, ssaoKernel.data());
		ssaoShader->SetMatrixUniform("projection", c->GetProjectionMatrix());
		ssaoGBuffer->BindGPosition(0);
		ssaoGBuffer->BindGNormal(1);
		ssaoNoiseTexture->Bind(2);
		PostProcess::PassThrough(ssaoShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FrameBuffer::UnBindTexture(0);
		FrameBuffer::UnBindTexture(1);
		FrameBuffer::UnBindTexture(2);

		// Do the SSAO Blur Pass
		ssaoBlurFBO->BindTarget();
		ShaderProgram* ssaoBlur = ShaderManager::GetShaderProgram("engine/shader/SSAOBlurPass");
		ssaoBlur->Bind();
		ssaoBlur->SetIntUniform("ssaoInput", 0);
		ssaoFBO->BindTexture(0);

		PostProcess::PassThrough(ssaoBlur);
		FrameBuffer::UnBindTexture(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_BLEND);
	}

	// Do normal render pass
	frameBufferRaw->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto& o : scene->objects)
		o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::Standard);

	// If MSAA is enabled then we need to blit to a Single Sample surface before doing PostProcess.
	if (msaaEnabled)
	{
		// blit it to a non-multisampled FBO for texture attachment compatiability.
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferRaw->GetID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferBlit->GetID());
		glBlitFramebuffer(0, 0, frameBufferRaw->GetWidth(), frameBufferRaw->GetHeight(), 0, 0, frameBufferBlit->GetWidth(), frameBufferBlit->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// bind output of first pass (raw render with no processing)
		frameBufferBlit->BindTexture(20);
	}
	else
		frameBufferRaw->BindTexture(20);

	ssaoBlurFBO->BindTexture(21); // This is a bit crap, SSAO is kind of half backed in to the above pipeline and a postprocess effect on the camera.
	c->RunPostProcess(frameBufferProcessed);

	// Stats
	renderTotalSamples[sampleIndex] = glfwGetTime() - renderLastFrameTime;
	renderLastFrameTime = glfwGetTime();
	
	sampleIndex++;
	if (sampleIndex == 100)
		sampleIndex = 0;
}

void SceneRenderer::RenderSceneShadowMaps(Scene* scene, ComponentCamera* camera)
{
	shadowMap->BindTarget();
	glClear(GL_DEPTH_BUFFER_BIT);
	// Generate shadow map VPM and camera pos
	glm::mat4 lightProjection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);
	glm::mat4 lightView = glm::lookAt(camera->GetWorldSpacePosition(), camera->GetWorldSpacePosition() + scene->GetSunDirection(), glm::vec3(0.0f, 0.0f, 1.0f));
	scene->lightSpaceMatrix = lightProjection * lightView;
	for (auto& o : scene->objects)
		o->Draw(scene->lightSpaceMatrix, { 0,0,0 }, Component::DrawMode::ShadowMapping);

	FrameBuffer::UnBindTarget();

}

void SceneRenderer::RenderSceneObjectPick(Scene* scene, ComponentCamera* c)
{
	objectPickBuffer->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 vpm = c->GetViewProjectionMatrix();
	glm::vec3 position = c->GetWorldSpacePosition();

	for (auto& o : scene->objects)
		o->Draw(vpm, position, Component::DrawMode::ObjectPicking);

	scene->objectPickedID = objectPickBuffer->GetObjectID(scene->requestedSelectionPosition.x,scene->requestedSelectionPosition.y);
	scene->requestedObjectSelection = false;
}

void SceneRenderer::RenderSceneGizmos(Scene* scene, ComponentCamera* c)
{
	// render light gizmos only to main 'editor' camera
	// quick wireframe rendering. Will later set up something that renders a quad billboard at the location or something.
	frameBufferProcessed->BindTarget();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gizmoShader->Bind();
	for (auto& light : scene->m_pointLights)
	{
		gizmoShader->SetVector3Uniform("gizmoColour", light.colour);

		vec3 localPosition, localRotation, localScale;
		localPosition = light.position;
		localScale = { 0.2f, 0.2f, 0.2f, };
		localRotation = { 90, 0, 0 };
		ImGuizmo::RecomposeMatrixFromComponents((float*)&localPosition, (float*)&localRotation, (float*)&localScale, (float*)&lightGizmo->transform);
		lightGizmo->Draw(c->GetViewProjectionMatrix(), c->GetWorldSpacePosition(), Component::DrawMode::Standard);
	}

	// Draw cameras (from gizmo list, all gizmos should move to here)
	gizmoShader->SetVector3Uniform("gizmoColour", { 1,1,1 });
	for (auto& o : scene->gizmos)
	{
		if (o == Scene::s_editorCamera->object)
			continue;

		o->Draw(c->GetViewProjectionMatrix(), c->GetWorldSpacePosition(), Component::DrawMode::Standard);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	FrameBuffer::UnBindTarget();
}

void SceneRenderer::DrawBackBuffer()
{
	frameBufferProcessed->BindTexture(20);
	PostProcess::PassThrough();
	FrameBuffer::UnBindTexture(20);
}

bool SceneRenderer::ShouldCull(vec3 position)
{
	return frustumCullingCamera->IsPointInFrustum(position, frustumCullingForgiveness);
}


void SceneRenderer::SetCullingCamera(int index)
{
	if (index < -1) index = -1;

	if (index >= (int)Scene::s_instance->componentCameras.size()) // cast the sizet to an int so it can be compared with a negatively signed number
		index = Scene::s_instance->componentCameras.size() - 1;

	frustumCullingCameraIndex = index;
	frustumCullingCamera = Scene::GetCameraByIndex(index);
}
#include "SceneRenderer.h"
#include "FrameBuffer.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "Window.h"
#include "UniformBuffer.h"

#include "ComponentCamera.h"
#include "PostProcess.h"

#include "ComponentLightPoint.h"

#include "MathUtils.h"

#include <random>
#include "GraphicsQuality.h"

// Stuff to move away
#include "SceneEditorCamera.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "ModelManager.h"
#include "MaterialManager.h"

#include "Input.h"

#include "LineRenderer.h"

#include "ComponentRenderer.h"
#include "Model.h"

bool SceneRenderer::fxaaEnabled = true; // leave false whilst refactoring depth prepass and AA.
bool SceneRenderer::ssaoEnabled = true;
bool SceneRenderer::bloomEnabled = true;
float SceneRenderer::ambient = 0.03f;
ComponentCamera* SceneRenderer::frustumCullingCamera = nullptr;
bool SceneRenderer::frustumCullingShowBounds = false;
float SceneRenderer::shadowMapRealtimeMaxDistance = 150;
int SceneRenderer::ssaoKernelTaps = 16;

FrameBuffer* SceneRenderer::pointlightCubeMapArrayStatic = nullptr;
FrameBuffer* SceneRenderer::pointlightCubeMapArrayDynamic = nullptr;

CameraFrustum* SceneRenderer::cullingFrustum = nullptr;
bool SceneRenderer::currentPassIsStatic = false;
bool SceneRenderer::currentPassIsSplit = false;

// Transparent rendering
vector<std::pair<ComponentRenderer*, int>> SceneRenderer::transparentCalls;

RenderBatch SceneRenderer::renderBatch;
SceneRenderer::Statistics SceneRenderer::statistic;

SceneRenderer::SceneRenderer()
{
	pointlightCubeMapArrayStatic = new FrameBuffer(FrameBuffer::Type::ShadowCubeMapArray);
	pointlightCubeMapArrayDynamic = new FrameBuffer(FrameBuffer::Type::ShadowCubeMapArray);


	// Initialise the Render Buffers
#pragma region Render Buffer creation
	gBuffer = new FrameBuffer(FrameBuffer::Type::gBuffer);
	TextureManager::s_instance->AddFrameBuffer("gBuffer", gBuffer);

	// 1 to render the scene to, a second for the final post processing effects to land to.
	frameBufferRaw = new FrameBuffer(FrameBuffer::Type::CameraTargetSingleSample);
	frameBufferRaw->MakePrimaryTarget();
	string FbName = "Scene Renderer_FrameBuffer";
	TextureManager::s_instance->AddFrameBuffer(FbName.c_str(), frameBufferRaw); // add the texture to the manager so we can bind it to meshes and stuff.

	// The final frame buffer after the post process of the camera has been performed.
	frameBufferProcessed = new FrameBuffer(FrameBuffer::Type::PostProcess);
	string processedFBName = FbName + "_Processed";
	TextureManager::s_instance->AddFrameBuffer(processedFBName.c_str(), frameBufferProcessed);
#pragma endregion

	// Intialise SSAO Configuration
#pragma region SSAO Configuration
	ssaoFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
	TextureManager::s_instance->AddFrameBuffer("SSAO", ssaoFBO);
	ssaoPingFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
	TextureManager::s_instance->AddFrameBuffer("SSAOPing", ssaoPingFBO);
	ssaoPongFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
	TextureManager::s_instance->AddFrameBuffer("SSAOPong", ssaoPongFBO);
	ssaoPostProcess = new FrameBuffer(FrameBuffer::Type::PostProcess);
	TextureManager::s_instance->AddFrameBuffer("SSAOPostProcess", ssaoPostProcess);

	// Generate the kernel - https://learnopengl.com/Advanced-Lighting/SSAO
	// Both from <random>
	ssaoGenerateKernel(ssaoKernelTaps);
	ssaoGenerateNoise();

#pragma endregion

	// Initialise Bloom
	bloomPingFBO = new FrameBuffer(FrameBuffer::Type::PostProcess, 0.25f); // quarter rez FBO for bloom, we get more blur for free.
	TextureManager::s_instance->AddFrameBuffer("BloomBlur", bloomPingFBO);
	bloomPongFBO = new FrameBuffer(FrameBuffer::Type::PostProcess, 0.25f);
	TextureManager::s_instance->AddFrameBuffer("BloomBlur2", bloomPongFBO);

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
	lightGizmoRenderer->submeshMaterials.resize(1);
	lightGizmoRenderer->submeshMaterials[0] = MaterialManager::GetMaterial("engine/model/materials/Gizmos.material");
	lightGizmoRenderer->receivesShadows = false;
	lightGizmo->components.push_back(lightGizmoRenderer);

	pointLightPositionBuffer = new UniformBuffer(sizeof(glm::vec4) * 50);
	pointLightColourBuffer = new UniformBuffer(sizeof(glm::vec4) * 50);

#pragma endregion

	LineRenderer::Initialise();
}

void SceneRenderer::SetQuality()
{
	switch (GraphicsQuality::m_quality)
	{
	case GraphicsQuality::Quality::High:
		shadowMapRealtimeMaxDistance = 300;
		ssaoKernelTaps = 16;
		break;
	case GraphicsQuality::Quality::Medium:
		shadowMapRealtimeMaxDistance = 150;
		ssaoKernelTaps = 8;
		break;
	case GraphicsQuality::Quality::Low:
		shadowMapRealtimeMaxDistance = 100;
		ssaoKernelTaps = 4;
		break;

	}
}

void SceneRenderer::DrawGUI()
{
	// Graphics Options - Abstract this as these options develop.
	int itemWidth = 150;
	float windowWidth = 315;
	float windowHeight = 620;
	float advancedStatisticsHeight = 90;
	ImGui::SetNextWindowSize({ windowWidth, windowHeight }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos({ 1200, 200 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Graphics");

	ImGui::DragFloat("Max ShadowMap Distance", &shadowMapRealtimeMaxDistance, 1, 0, 50000);
	float averageTotalRenderTime = 0.0f;
	for (int i = 0; i < 100; i++)
		averageTotalRenderTime += renderTotalSamples[i];
	averageTotalRenderTime /= 100.0f;
	ImGui::BeginDisabled();
		ImGui::InputFloat("MS Average", &averageTotalRenderTime, 0, 0, "%0.6f", ImGuiInputTextFlags_ReadOnly);
		ImGui::PlotLines("", renderTotalSamples, 100, 0, "0 to 16.6ms", 0, 16.666, { 300,100 });
	ImGui::EndDisabled();
	if (ImGui::Checkbox("Advanced Statistics", &showAdvancedStats))
	{
		if (showAdvancedStats)
			ImGui::SetWindowSize({ windowWidth,windowHeight + advancedStatisticsHeight });
	}
	if (showAdvancedStats) DrawStatistics();
	ImGui::Text("Configuration");
	ImGui::PushItemWidth(itemWidth);
		ImGui::DragFloat("PBR Ambient", &ambient, 0.02, 0, 1);
	ImGui::PopItemWidth();
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
		ImGui::PushItemWidth(itemWidth);
			if (ImGui::InputInt("Camera", &frustumCullingCameraIndex, 1))
				SetCullingCamera(frustumCullingCameraIndex);
			ImGui::Checkbox("Show Bounding Boxes", &frustumCullingShowBounds);
		ImGui::Unindent();
	}

	if (ImGui::Checkbox("FXAA", &fxaaEnabled))
	{
		for (auto camera : Scene::s_instance->componentCameras)
		{
			if (fxaaEnabled) camera->AddPostProcess("engine/shader/postProcess/FXAA");
			else camera->RemovePostProcess("engine/shader/postProcess/FXAA");
		}

		if (Scene::s_editorCamera)
		{
			auto camera = Scene::s_editorCamera->camera;
			if (fxaaEnabled) camera->AddPostProcess("engine/shader/postProcess/FXAA");
			else camera->RemovePostProcess("engine/shader/postProcess/FXAA");
		}
	}

	ImGui::Checkbox("Bloom", &bloomEnabled);
	ImGui::Indent();
	ImGui::PushItemWidth(itemWidth);
		ImGui::DragInt("Bloom Taps", &bloomBlurTaps, 1, 1, 20, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopItemWidth();
	ImGui::Unindent();
	ImGui::Checkbox("SSAO", &ssaoEnabled);
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
		ImGui::PushID("Kernel Taps");
		ImGui::Text("Kernel Taps");
		if (ImGui::InputInt("", &ssaoKernelTaps))
		{
			ssaoKernelTaps = glm::clamp(ssaoKernelTaps, 0, 64); // hardcoded as 64 as a max. matches the magic number in the frag shader.
			ssaoGenerateKernel(ssaoKernelTaps);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			ssaoKernelTaps = 64;
			ssaoGenerateKernel(ssaoKernelTaps);
		}
		ImGui::PopID();
		ImGui::Checkbox("Blur?", &ssaoBlur);
		ImGui::Checkbox("2-Pass Gaussian?", &ssaoGaussianBlur);
		ImGui::Text("");
		ImGui::Unindent();
	}
	ImGui::PopItemWidth();
	if (ImGui::Button("Reload Shaders"))
	{
		ShaderManager::RecompileAllShaderPrograms();
		
		// Reconfigure the SSAO Kernel as the shader got wiped.
		ShaderProgram* ssaoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOTermPass");
		ssaoShader->Bind();
		ssaoShader->SetFloat3ArrayUniform("samples", ssaoKernelTaps, ssaoKernel.data());
	}

	ImGui::End();
}

void SceneRenderer::DrawStatistics()
{
	ImGui::BeginDisabled();
	ImGui::Indent();
	ImGui::PushItemWidth(100);
		ImGui::InputInt("Shader Batches", &statistic.shaderBatches,0,0,ImGuiInputTextFlags_ReadOnly);
		ImGui::InputInt("Material Batches", &statistic.materialBatches, 0, 0, ImGuiInputTextFlags_ReadOnly);
		ImGui::InputInt("Draw Calls", &statistic.drawCalls, 0, 0, ImGuiInputTextFlags_ReadOnly);
		ImGui::InputInt("Tris", &statistic.tris, 0, 0, ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();
	ImGui::Unindent();
	ImGui::EndDisabled();
}

void SceneRenderer::DrawShadowCubeMappingGUI()
{
	ImGui::Begin("Shadow Cube Map Devvoooo");
	ImGui::DragFloat("FOV", &FOV);
	ImGui::DragFloat("Aspect", &aspect);
	ImGui::DragFloat("Near", &nearNum);
	ImGui::DragFloat("Far", &farNum);

	ImGui::Text("");
	ImGui::DragFloat3("Pos X", &cubeMapDirections[0].target.x,1, -1, 1);
	ImGui::DragFloat3("Pos X Up", &cubeMapDirections[0].up.x ,1, -1, 1);
	ImGui::DragFloat3("Neg X", &cubeMapDirections[1].target.x,1, -1, 1);
	ImGui::DragFloat3("Neg X Up", &cubeMapDirections[1].up.x ,1, -1, 1);
	ImGui::DragFloat3("Pos Y", &cubeMapDirections[2].target.x,1, -1, 1);
	ImGui::DragFloat3("Pos Y Up", &cubeMapDirections[2].up.x ,1, -1, 1);
	ImGui::DragFloat3("Neg Y", &cubeMapDirections[3].target.x,1, -1, 1);
	ImGui::DragFloat3("Neg Y Up", &cubeMapDirections[3].up.x ,1, -1, 1);
	ImGui::DragFloat3("Pos Z", &cubeMapDirections[4].target.x,1, -1, 1);
	ImGui::DragFloat3("Pos Z Up", &cubeMapDirections[4].up.x ,1, -1, 1);
	ImGui::DragFloat3("Neg Z", &cubeMapDirections[5].target.x,1, -1, 1);
	ImGui::DragFloat3("Neg Z Up", &cubeMapDirections[5].up.x ,1, -1, 1);
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

void SceneRenderer::Prepare(Scene* scene)
{
	scene->UpdatePointLightData();
}

void SceneRenderer::RenderScene(Scene* scene, ComponentCamera* c)
{
	Prepare(scene);

	// Set up buffers for da shaders - move this in to material batching.
	ShaderProgram* shader = ShaderManager::GetShaderProgram("engine/shader/PBR");
	//shader->Bind();
	shader->SetUniformBlockIndex("pointLightPositionBuffer", 1);
	shader->SetUniformBlockIndex("pointLightColourBuffer", 2);

	shader = ShaderManager::GetShaderProgram("engine/shader/PBRSkinned");
	//shader->Bind();
	shader->SetUniformBlockIndex("pointLightPositionBuffer", 1);
	shader->SetUniformBlockIndex("pointLightColourBuffer", 2);

	shader = ShaderManager::GetShaderProgram("engine/shader/Lambert");
	//shader->Bind();
	shader->SetUniformBlockIndex("pointLightPositionBuffer", 1);
	shader->SetUniformBlockIndex("pointLightColourBuffer", 2);

	shader = ShaderManager::GetShaderProgram("engine/shader/LambertSkinned");
	//shader->Bind();
	shader->SetUniformBlockIndex("pointLightPositionBuffer", 1);
	shader->SetUniformBlockIndex("pointLightColourBuffer", 2);
	pointLightPositionBuffer->SendData(Scene::GetPointLightPositions());
	pointLightPositionBuffer->Bind(1);
	pointLightColourBuffer->SendData(Scene::GetPointLightColours());
	pointLightColourBuffer->Bind(2);


	// Shadow map dev stuff
	// Bind recently rendered shadowmap.
	//shadowMap->BindTexture(20);
	//shadowMapDevOutput->BindTarget();
	//PostProcess::PassThrough(depthMapOutputShader);

	//shadowMap->BindTexture(5);

	// Render any 'static' cubemaps that need to (They have been newly created, or the list of static and dynamic objects have changed)
	glDisable(GL_BLEND);
	currentPassIsStatic = true;
	currentPassIsSplit = true;
	RenderSceneShadowCubeMaps(scene);
	pointLightShadowMapsStaticDirty = false;

	// Render all dynamic objects.
	currentPassIsStatic = false;
	currentPassIsSplit = true;
	RenderSceneShadowCubeMaps(scene);
	currentPassIsSplit = false;


	// Prepare to render scene from our main camera
	if (frustumCullingEnabled) cullingFrustum = &frustumCullingCamera->frustum;
	else cullingFrustum = nullptr;
	vec3 cameraPosition = c->GetWorldSpacePosition();
	
	// Render Scene SSAO & Depth Pre-Pass

	
	// Build G Buffer
	gBuffer->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderBatch.SetRenderPass(RenderBatch::RenderPass::gBuffer);

	for (auto& o : scene->objects)
		o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::SSAOgBuffer);

	renderBatch.SetCameraMatricies(c);
	renderBatch.DrawBatches();
	renderBatch.ClearBatches();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// SSAO
	FrameBuffer* blurBufferUsed = nullptr; // store this outside so we can assign it based on whether we blurred, and which blur buffer we used.
	if (ssaoEnabled)
	{
		// Do the SSAO pass
		ssaoFBO->BindTarget();
		glClear(GL_COLOR_BUFFER_BIT);

		ShaderProgram* ssaoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOTermPass");
		ssaoShader->Bind();
		ssaoShader->SetFloatUniform("radius", ssaoRadius);
		ssaoShader->SetFloatUniform("bias", ssaoBias);
		glm::vec2 screenSize = Window::GetViewPortSize();
		ssaoShader->SetVector2Uniform("screenSize", screenSize);
		ssaoShader->SetIntUniform("kernelTaps", ssaoKernelTaps);

		ssaoShader->SetIntUniform("gPosition", 0);
		ssaoShader->SetIntUniform("gNormal", 1);
		ssaoShader->SetIntUniform("texNoise", 2);
		ssaoShader->SetMatrixUniform("projection", c->GetProjectionMatrix());
		gBuffer->BindGPosition(0);
		gBuffer->BindGNormal(1);
		ssaoNoiseTexture->Bind(2);
		PostProcess::PassThrough(true);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FrameBuffer::UnBindTexture(0);
		FrameBuffer::UnBindTexture(1);
		FrameBuffer::UnBindTexture(2);

		// Do the SSAO Blur Pass
		if (ssaoBlur)
		{
			ssaoPingFBO->BindTarget();
			if (ssaoGaussianBlur)
			{
				ShaderProgram* ssaoBlur = ShaderManager::GetShaderProgram("engine/shader/SSAOGaussianBlurPass");
				ssaoBlur->Bind();
				ssaoBlur->SetIntUniform("image", 0);
				ssaoBlur->SetIntUniform("horizontal", true);
				ssaoFBO->BindTexture(0);
				PostProcess::PassThrough(true);
				FrameBuffer::UnBindTexture(0);

				ssaoPingFBO->BindTexture(0);
				ssaoPongFBO->BindTarget();
				ssaoBlur->SetIntUniform("horizontal", false);
				PostProcess::PassThrough(true);
				FrameBuffer::UnBindTexture(0);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				blurBufferUsed = ssaoPongFBO;
			}
			else
			{
				ShaderProgram* ssaoBlur = ShaderManager::GetShaderProgram("engine/shader/SSAOBlurPass");
				ssaoBlur->Bind();
				ssaoBlur->SetIntUniform("ssaoInput", 0);
				ssaoFBO->BindTexture(0);
				PostProcess::PassThrough(true);
				FrameBuffer::UnBindTexture(0);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				blurBufferUsed = ssaoPingFBO;
			}
		}
		else blurBufferUsed = ssaoFBO;
	}

	// Render Scene Opaque Pass
	frameBufferRaw->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT);
	static const vec3 nothing = { 0, 0, 0 };
	glClearTexImage(frameBufferRaw->m_emissiveTexID, 0, GL_RGB, GL_FLOAT, &nothing);

	gBuffer->BindAsDepthAttachment();
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);

	// Build render batches
	renderBatch.SetRenderPass(RenderBatch::RenderPass::Opaque);
	for (auto& o : scene->objects)
		o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::BatchedOpaque);

	renderBatch.SetCameraMatricies(c);
	renderBatch.DrawBatches();
	renderBatch.ClearBatches();

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);


	// Apply SSAO buffer if it is enabled.
	frameBufferCurrent = frameBufferRaw;
	frameBufferCurrent->BindTexture(20);
	if (ssaoEnabled)
	{
		// SSAO application
		ShaderProgram* SSAOshader = ShaderManager::GetShaderProgram("engine/shader/postProcess/SSAO");
		SSAOshader->Bind();
		SSAOshader->SetIntUniform("frame", 20);
		SSAOshader->SetIntUniform("SSAO", 21);
		blurBufferUsed->BindTexture(21); // This is a bit crap, SSAO is kind of half baked in to the above pipeline and a postprocess effect on the camera.
		frameBufferCurrent = ssaoPostProcess;
		frameBufferCurrent->BindTarget();
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		PostProcess::PassThrough(true);
		frameBufferCurrent->BindTexture(20);
	}

	// Render Transparent Pass
	gBuffer->BindAsDepthAttachment();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, frameBufferRaw->m_emissiveTexID, 0);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	RenderTransparent(scene, c);

	if (bloomEnabled)

	{	// Blur the emission texture to create a bloom texture
		ShaderProgram* blurShader = ShaderManager::GetShaderProgram("engine/shader/GaussianBlurPass");
		blurShader->Bind();
		frameBufferRaw->BindEmission(0);
		FrameBuffer* blur = frameBufferRaw;
		for (int i = 0; i < bloomBlurTaps; i++)
		{
			blurShader->SetIntUniform("image", 0);
			blurShader->SetIntUniform("horizontal", true);
			vec2 outputSize = { bloomPingFBO->GetWidth(), bloomPingFBO->GetHeight() };
			blurShader->SetVector2Uniform("outputSize", outputSize);
			bloomPingFBO->BindTarget();
			PostProcess::PassThrough(true);
			FrameBuffer::UnBindTexture(0);

			bloomPingFBO->BindTexture(0);
			bloomPongFBO->BindTarget();
			blurShader->SetIntUniform("horizontal", false);
			PostProcess::PassThrough(true);
			
			// ready for next loop.
			if(i != bloomBlurTaps -1) bloomPongFBO->BindTexture(0);
		}
		FrameBuffer::UnBindTexture(0);

		// Bloom application
		ShaderProgram* BloomShader = ShaderManager::GetShaderProgram("engine/shader/postProcess/Bloom");
		BloomShader->Bind();
		BloomShader->SetIntUniform("frame", 20);
		BloomShader->SetIntUniform("Bloom", 21);
		bloomPongFBO->BindTexture(21);
		frameBufferCurrent->BindTarget();
		PostProcess::PassThrough(true);
		frameBufferCurrent->BindTexture(20);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// Run cameras post process effects.
	c->RunPostProcess(frameBufferProcessed);

	CleanUp(scene);

	// Stats
	renderTotalSamples[sampleIndex] = (glfwGetTime() - renderLastFrameTime) * 1000.0f; // convert from seconds to milliseconds.
	renderLastFrameTime = glfwGetTime();
	
	sampleIndex++;
	if (sampleIndex == 100)
		sampleIndex = 0;
}

void SceneRenderer::RenderSceneShadowCubeMaps(Scene* scene)
{
	// if not prepass then we blit the prepasses to the regular pass, then those are the targets we're rendering (adding) too
	//glDisable(GL_BLEND);


	if (!currentPassIsStatic && scene->GetNumPointLights() > 0)
	{
		glCopyImageSubData(pointlightCubeMapArrayStatic->m_depthID, GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, pointlightCubeMapArrayDynamic->m_depthID, GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, pointlightCubeMapArrayStatic->m_width, pointlightCubeMapArrayStatic->m_height, 6 * scene->m_pointLightComponents.size());
	}

	// Render shadow maps for each point light
	for (int light = 0; light < scene->m_pointLightComponents.size(); light++)
	{
		if (currentPassIsStatic && // We're doing a static pass
			(!scene->m_pointLightComponents[light]->GetComponentParentObject()->wasDirtyTransform && // The light hasnt moved
				!scene->rendererShouldRefreshStaticMaps && // 
				!pointLightShadowMapsStaticDirty)) continue;

		vec3 lightPosition = scene->m_pointLightComponents[light]->GetComponentParentObject()->GetWorldSpacePosition();
		// Render to each side of the cube face.
		for (int i = 0; i < 6; i++)
		{
			if (currentPassIsStatic)
			{
				pointlightCubeMapArrayStatic->BindTarget(i, light);
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			else
			{
				pointlightCubeMapArrayDynamic->BindTarget(i, light);
			}
			
			// Build the cubeMap projection and frustum
			mat4 projection = glm::perspective(glm::radians(FOV), aspect, nearNum, farNum);
			mat4 view = glm::lookAt(lightPosition, lightPosition + cubeMapDirections[i].target, cubeMapDirections[i].up);
			mat4 lightVP = projection * view;
			CameraFrustum cubeMapFrustum = CameraFrustum::GetFrustumFromVPMatrix(lightVP);
			cullingFrustum = &cubeMapFrustum;

			// Render scene
			for (auto& o : scene->objects)
				o->Draw(lightVP, lightPosition, Component::DrawMode::ShadowCubeMapping);

		}
	}
	if (scene->rendererShouldRefreshStaticMaps) scene->rendererShouldRefreshStaticMaps = false;
	FrameBuffer::UnBindTarget();
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

void SceneRenderer::RenderTransparent(Scene* scene, ComponentCamera* camera)
{
	if (transparentCalls.size() > 0)
	{
		// set up blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		// do not cull any faces
		glDisable(GL_CULL_FACE);

		// depth
		glDepthMask(GL_FALSE); // Disable writing to the depth buffer

		// framebuffer
		//frameBufferCurrent->BindTarget();
		
		//gBuffer->BindAsDepthAttachment();
		// draw all components w/ transparent mode
		glm::mat4 pv = camera->GetViewProjectionMatrix();
		glm::vec3 pos = camera->GetWorldSpacePosition();

		// probably do some sorting on these calls maybe, depending on blend mode?

		// Soujaboy draw 'em
		for (auto& c : transparentCalls)
		{
			ComponentRenderer* renderer = c.first;
			int subMeshIndex = c.second;
			renderer->material = renderer->submeshMaterials[subMeshIndex];
			renderer->BindShader();
			renderer->ApplyMaterials();
			renderer->BindMatricies(pv, pos);
			renderer->model->DrawSubMesh(subMeshIndex);
		}

		// clean up
		transparentCalls.resize(0);
	}
	// reenable depth writing
	glDepthMask(GL_TRUE);
	// reenable face culling
	glEnable(GL_CULL_FACE);

	// disabling blending
	glDisable(GL_BLEND);
}

void SceneRenderer::RenderSceneGizmos(Scene* scene, ComponentCamera* c)
{
	// render light gizmos only to main 'editor' camera
	// quick wireframe rendering. Will later set up something that renders a quad billboard at the location or something.
	frameBufferProcessed->BindTarget();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gizmoShader->Bind();
	for (auto& pointLight : scene->m_pointLightComponents)
	{
		gizmoShader->SetVector3Uniform("gizmoColour", pointLight->colour);

		vec3 localPosition, localRotation, localScale;
		localPosition = pointLight->GetComponentParentObject()->GetWorldSpacePosition();
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

void SceneRenderer::RenderLines(ComponentCamera* camera)
{
	// Line Renderer
	frameBufferProcessed->BindTarget();
	LineRenderer::Render(camera->GetViewProjectionMatrix());
	FrameBuffer::UnBindTarget();
}

void SceneRenderer::CleanUp(Scene* scene)
{
}

void SceneRenderer::DrawBackBuffer()
{
	frameBufferProcessed->BindTexture(20);
	PostProcess::PassThrough();
	FrameBuffer::UnBindTexture(20);
}

bool SceneRenderer::compareIndexDistancePair(std::pair<int, float> a, std::pair<int, float> b)
{
		return a.second < b.second;
}

bool SceneRenderer::ShouldCull(const glm::vec3* points)
{
	for (int face = 0; face < 5; face++) // dont do far plane
	{
		bool pointInPlane = false;
		for (int i = 0; i < 8; i++)
		{
			if (CameraFrustum::IsPointInPlane(points[i], cullingFrustum->faces[face]))
			{
				pointInPlane = true;
				break;
			}
		}
		if (!pointInPlane) return true;
	}
	return false;
}

void SceneRenderer::SetCullingCamera(int index)
{
	if (index < -1) index = -1;

	if (index >= (int)Scene::s_instance->componentCameras.size()) // cast the sizet to an int so it can be compared with a negatively signed number
		index = Scene::s_instance->componentCameras.size() - 1;

	frustumCullingCameraIndex = index;
	frustumCullingCamera = Scene::GetCameraByIndex(index);
}

void SceneRenderer::ssaoGenerateKernel(int size)
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	unsigned int kernalSize = size;
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
	}
	// Upload the Kernel
	ShaderProgram* ssaoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOTermPass");
	ssaoShader->Bind();
	ssaoShader->SetFloat3ArrayUniform("samples", size, ssaoKernel.data());
}

void SceneRenderer::ssaoGenerateNoise()
{
	// Generate SSAO Noise
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
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
}
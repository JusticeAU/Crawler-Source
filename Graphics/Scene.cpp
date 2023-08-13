#include "Scene.h"
#include "FileUtils.h"
#include "ModelManager.h"
#include "Model.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "AudioManager.h"

#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Window.h"
#include "MeshManager.h"

#include "Camera.h"

#include "MathUtils.h"
#include "LogUtils.h"
#include "PostProcess.h"

#include "serialisation.h"

#include <fstream>
#include <filesystem>
#include <random>
namespace fs = std::filesystem;

Scene::Scene()
{
	// Create light pos and col arrays for sending to lit phong shader.
	m_pointLightPositions = new vec3[MAX_LIGHTS];
	m_pointLightColours = new vec3[MAX_LIGHTS];

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

	// Add editor camera to list of cameras and set our main camera to be it.
	cameras.push_back(Camera::s_instance->GetFrameBufferProcessed());
	outputCameraFrameBuffer = cameras[0];

	// Object picking dev - Might be able to refactor this to be something that gets attached to a camera. It needs to be used by both scene editor camera, but also 'in game' camera - not unreasonable for these to be seperate implementations though.
	if (objectPickBuffer == nullptr)
	{
		objectPickBuffer = new FrameBuffer(FrameBuffer::Type::ObjectPicker);
		TextureManager::s_instance->AddFrameBuffer("Objecting Picking Buffer", objectPickBuffer);
	}

	// Shadow Mapping dev - This will get refactored in to something more robust once I've set up PBR.
	shadowMap = new FrameBuffer(FrameBuffer::Type::ShadowMap);
	shadowMapDevOutput = new FrameBuffer(FrameBuffer::Type::PostProcess);
	depthMapOutputShader = ShaderManager::GetShaderProgram("engine/shader/zzShadowMapDev");

	// SSAO Dev
	if (ssao_gBuffer == nullptr)
	{
		// Initialise the buffers
		ssao_gBuffer = new FrameBuffer(FrameBuffer::Type::SSAOgBuffer);
		TextureManager::s_instance->AddFrameBuffer("gBuffer", ssao_gBuffer);
		ssao_ssaoFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
		TextureManager::s_instance->AddFrameBuffer("SSAO", ssao_ssaoFBO);
		ssao_ssaoBlurFBO = new FrameBuffer(FrameBuffer::Type::SSAOColourBuffer);
		TextureManager::s_instance->AddFrameBuffer("SSAOBlur", ssao_ssaoBlurFBO);
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
		ssao_noiseTexture = new Texture();
		ssao_noiseTexture->CreateSSAONoiseTexture(ssaoNoise.data());
		TextureManager::AddTexture(ssao_noiseTexture);
	}
}
Scene::~Scene()
{
	for (auto o : objects)
	{
		o->DeleteAllChildren();
		delete o;
	}

	objects.clear();
}

void Scene::Init()
{
	/*s_instance = new Scene();
	s_instances.emplace("default", s_instance);*/
}

Scene* Scene::NewScene(string name)
{
	Scene* scene = new Scene();
	scene->sceneName = name;
	s_instances.emplace(name, scene);
	return scene;
}

void Scene::Update(float deltaTime)
{
	UpdateInputs();

	// Update all Objects
	for (auto o : objects)
		o->Update(deltaTime);

	// Update point light arrays - right now we send this in as ivec and vec4s - should turn this in to a more dynamic uniform buffer.
	for (int i = 0; i < m_pointLights.size(); i++)
	{
		m_pointLightColours[i] = m_pointLights[i].colour * m_pointLights[i].intensity;
		m_pointLightPositions[i] = m_pointLights[i].position;
	}

}
void Scene::UpdateInputs()
{
	/*if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Pressed() && Input::Keyboard(GLFW_KEY_D).Down())
		DuplicateObject(selectedObject);*/

	// Disable Image render based object picking for now.
	/*
	if (Input::Mouse(0).Down() && !ImGuizmo::IsOver())
	{
		requestedObjectSelection = true;
		requestedSelectionPosition = Input::GetMousePosPixel();
	}
	*/
	
}

void Scene::Render()
{
	//RenderShadowMaps();
	if (requestedObjectSelection)
		RenderObjectPicking();
	if(cameraIndex != 0)
		RenderSceneCameras();
	else
		RenderEditorCamera();
	if(drawGizmos)
		DrawGizmos();

	double endTime = glfwGetTime();
	renderTime = endTime - lastRenderTimeStamp;
	lastRenderTimeStamp = endTime;
}
void Scene::RenderShadowMaps()
{
	shadowMap->BindTarget();
	glClear(GL_DEPTH_BUFFER_BIT);
	// Generate shadow map VPM and camera pos
	lightProjection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);
	lightView = glm::lookAt(Camera::s_instance->GetPosition(),
		Camera::s_instance->GetPosition() + Scene::GetSunDirection(),
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
	for (auto& o : objects)
		o->Draw(lightSpaceMatrix, { 0,0,0 }, Component::DrawMode::ShadowMapping);

	FrameBuffer::UnBindTarget();
}
void Scene::RenderSceneCameras()
{
	// Bind recently rendered shadowmap.
	//shadowMap->BindTexture(20);
	//shadowMapDevOutput->BindTarget();
	//PostProcess::PassThrough(depthMapOutputShader);

	//shadowMap->BindTexture(5);
	// for each camera in each object, draw to that cameras frame buffer

	ComponentCamera* c = componentCameras[cameraIndex - 1];
	c->UpdateViewProjectionMatrix();
	vec3 cameraPosition = c->GetWorldSpacePosition();

	if (ssao_enabled)
	{
		// Build G Buffer for SSAO with a Geomerty Pass.
		ssao_gBuffer->BindTarget();
		ShaderProgram* ssaoGeoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOGeometryPass");
		ssaoGeoShader->Bind();
		ssaoGeoShader->SetMatrixUniform("view", c->GetViewMatrix());
		ssaoGeoShader->SetMatrixUniform("projection", c->GetProjectionMatrix());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind the shader for it - just use static for now but will need something for skinned.
		glDisable(GL_BLEND);
		for (auto& o : objects)
			o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::SSAOgBuffer);
		glEnable(GL_BLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Do the SSAO pass
		ssao_ssaoFBO->BindTarget();
		glClear(GL_COLOR_BUFFER_BIT);

		ShaderProgram* ssaoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOTermPass");
		ssaoShader->Bind();
		ssaoShader->SetFloatUniform("radius", ssao_radius);
		ssaoShader->SetFloatUniform("bias", ssao_bias);
		vec2 screenSize = Window::GetViewPortSize();
		ssaoShader->SetVector2Uniform("screenSize", screenSize);

		glDisable(GL_BLEND);
		ssaoShader->SetIntUniform("gPosition", 0);
		ssaoShader->SetIntUniform("gNormal", 1);
		ssaoShader->SetIntUniform("texNoise", 2);

		ssaoShader->SetFloat3ArrayUniform("samples", 64, ssaoKernel.data());
		ssaoShader->SetMatrixUniform("projection", c->GetProjectionMatrix());
		ssao_gBuffer->BindGPosition(0);
		ssao_gBuffer->BindGNormal(1);
		ssao_noiseTexture->Bind(2);
		PostProcess::PassThrough(ssaoShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FrameBuffer::UnBindTexture(0);
		FrameBuffer::UnBindTexture(1);
		FrameBuffer::UnBindTexture(2);

		// Do the SSAO Blur Pass
		ssao_ssaoBlurFBO->BindTarget();
		ShaderProgram* ssaoBlur = ShaderManager::GetShaderProgram("engine/shader/SSAOBlurPass");
		ssaoBlur->Bind();
		ssaoBlur->SetIntUniform("ssaoInput", 0);
		ssao_ssaoFBO->BindTexture(0);

		PostProcess::PassThrough(ssaoBlur);
		FrameBuffer::UnBindTexture(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Draw

		glEnable(GL_BLEND);
	}

	// Do normal render pass
	c->SetAsRenderTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto& o : objects)
		o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::Standard);
	c->RunPostProcess();
}
void Scene::RenderObjectPicking()
{
	objectPickBuffer->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glm::mat4 vpm;
	glm::vec3 position;
	if (cameraIndex == 0)
	{
		vpm = Camera::s_instance->GetMatrix();
		position = Camera::s_instance->GetPosition();
	}
	else
	{
		vpm = componentCameras[0]->GetViewProjectionMatrix();
		position = componentCameras[0]->GetWorldSpacePosition();
	}

	for (auto& o : objects)
		o->Draw(vpm, position, Component::DrawMode::ObjectPicking);

	//SetSelectedObject(objectPickBuffer->GetObjectID(requestedSelectionPosition.x, requestedSelectionPosition.y)); // old method for transform gizmo
	requestedSelectionPosition = Input::GetMousePosPixel();
	objectPickedID = objectPickBuffer->GetObjectID(requestedSelectionPosition.x, requestedSelectionPosition.y);
	std::cout << objectPickedID << std::endl;
	requestedObjectSelection = false;
}
void Scene::RenderEditorCamera()
{
	if (ssao_enabled)
	{
		// Build G Buffer for SSAO with a Geomerty Pass.
		ssao_gBuffer->BindTarget();
		ShaderProgram* ssaoGeoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOGeometryPass");
		ssaoGeoShader->Bind();
		ssaoGeoShader->SetMatrixUniform("view", Camera::s_instance->GetView());
		ssaoGeoShader->SetMatrixUniform("projection", Camera::s_instance->GetProjection());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		vec3 cameraPosition = Camera::s_instance->GetPosition();
		// bind the shader for it - just use static for now but will need something for skinned.
		glDisable(GL_BLEND);
		for (auto& o : objects)
			o->Draw(Camera::s_instance->GetMatrix(), cameraPosition, Component::DrawMode::SSAOgBuffer);
		glEnable(GL_BLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Do the SSAO pass
		ssao_ssaoFBO->BindTarget();
		glClear(GL_COLOR_BUFFER_BIT);

		ShaderProgram* ssaoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOTermPass");
		ssaoShader->Bind();
		ssaoShader->SetFloatUniform("radius", ssao_radius);
		ssaoShader->SetFloatUniform("bias", ssao_bias);
		vec2 screenSize = Window::GetViewPortSize();
		ssaoShader->SetVector2Uniform("screenSize", screenSize);

		glDisable(GL_BLEND);
		ssaoShader->SetIntUniform("gPosition", 0);
		ssaoShader->SetIntUniform("gNormal", 1);
		ssaoShader->SetIntUniform("texNoise", 2);

		ssaoShader->SetFloat3ArrayUniform("samples", 64, ssaoKernel.data());
		ssaoShader->SetMatrixUniform("projection", Camera::s_instance->GetProjection());
		ssao_gBuffer->BindGPosition(0);
		ssao_gBuffer->BindGNormal(1);
		ssao_noiseTexture->Bind(2);
		PostProcess::PassThrough(ssaoShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FrameBuffer::UnBindTexture(0);
		FrameBuffer::UnBindTexture(1);
		FrameBuffer::UnBindTexture(2);

		// Do the SSAO Blur Pass
		ssao_ssaoBlurFBO->BindTarget();
		ShaderProgram* ssaoBlur = ShaderManager::GetShaderProgram("engine/shader/SSAOBlurPass");
		ssaoBlur->Bind();
		ssaoBlur->SetIntUniform("ssaoInput", 0);
		ssao_ssaoFBO->BindTexture(0);

		PostProcess::PassThrough(ssaoBlur);
		FrameBuffer::UnBindTexture(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Draw

		glEnable(GL_BLEND);

	}
	Camera::s_instance->GetFrameBuffer()->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& o : objects)
		o->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::Standard);
	FrameBuffer::UnBindTarget();

	FrameBuffer* editorReadFBO;
	if (MSAAEnabled)
	{
		// blit it to a non-multisampled FBO for texture attachment compatiability.
		FrameBuffer* editorFBO = Camera::s_instance->GetFrameBuffer();
		editorReadFBO = Camera::s_instance->GetFrameBufferBlit();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, editorFBO->GetID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, editorReadFBO->GetID());
		glBlitFramebuffer(0, 0, editorFBO->GetWidth(), editorFBO->GetHeight(), 0, 0, editorReadFBO->GetWidth(), editorReadFBO->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
	else
		editorReadFBO = Camera::s_instance->GetFrameBuffer();

	if (ssao_enabled)
	{
		// draw combine with SSAO texture
		Camera::s_instance->GetFrameBufferProcessed()->BindTarget();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		ShaderProgram* ssao = ShaderManager::GetShaderProgram("engine/shader/postProcess/SSAO");
		ssao->Bind();
		editorReadFBO->BindTexture(20);
		ssao_ssaoBlurFBO->BindTexture(21);
		ssao->SetIntUniform("frame", 20);
		ssao->SetIntUniform("SSAO", 21);

		PostProcess::PassThrough(ssao);
		FrameBuffer::UnBindTarget();
		FrameBuffer::UnBindTexture(20);
		FrameBuffer::UnBindTexture(21);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		FrameBuffer* editorFBO = Camera::s_instance->GetFrameBuffer();
		FrameBuffer* editorProcessedFBO = Camera::s_instance->GetFrameBufferProcessed();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, editorFBO->GetID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, editorProcessedFBO->GetID());
		glBlitFramebuffer(0, 0, editorFBO->GetWidth(), editorFBO->GetHeight(), 0, 0, editorProcessedFBO->GetWidth(), editorProcessedFBO->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
}

void Scene::DrawGizmos()
{
	// render light gizmos only to main 'editor' camera
	// quick wireframe rendering. Will later set up something that renders a quad billboard at the location or something.
	Camera::s_instance->GetFrameBufferProcessed()->BindTarget();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gizmoShader->Bind();
	for (auto &light : m_pointLights)
	{
		gizmoShader->SetVector3Uniform("gizmoColour", light.colour);

		vec3 localPosition, localRotation, localScale;
		localPosition = light.position;
		localScale = { 0.2f, 0.2f, 0.2f, };
		localRotation = { 0, 0, 0 };
		ImGuizmo::RecomposeMatrixFromComponents((float*)&localPosition, (float*)&localRotation, (float*)&localScale, (float*)&lightGizmo->transform);
		lightGizmo->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::Standard);
	}

	// Draw cameras (from gizmo list, all gizmos should move to here)
	gizmoShader->SetVector3Uniform("gizmoColour", { 1,1,1 });
	for (auto &o : gizmos)
	{
		o->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::Standard);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	Camera::s_instance->BlitFrameBuffer();
	FrameBuffer::UnBindTarget();
}
void Scene::DrawCameraToBackBuffer()
{
	outputCameraFrameBuffer->BindTexture(20);
	PostProcess::PassThrough();
	FrameBuffer::UnBindTexture(20);
}
void Scene::DrawGUI()
{	
	/*if (dungeonEditingEnabled)
		dungeonEditor.DrawGUI();
	else if (isArtTesting)
		artTester.DrawGUI();*/
	//else
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

		ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		if (ImGui::Button("Save"))
			SaveJSON();
		ImGui::SameLine();
		if (ImGui::Button("Load"))
			ImGui::OpenPopup("popup_load_scene");

		// Draw scene file list if requested
		if (ImGui::BeginPopup("popup_load_scene"))
		{
			ImGui::SameLine();
			ImGui::SeparatorText("Scene Name");
			for (auto d : fs::recursive_directory_iterator("scenes"))
			{
				if (d.path().has_extension() && d.path().extension() == ".scene")
				{
					string foundSceneName = d.path().filename().string();
					string foundScenePath = d.path().relative_path().string();
					if (ImGui::Selectable(foundScenePath.c_str()))
					{
						sceneFilename = foundSceneName;
						LoadJSON(); // TODO - sanitize these paths properly 
					}
				}
			}
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::InputText("Name", &sceneFilename);
		ImGui::BeginDisabled();
		int obj = selectedObjectID;
		ImGui::InputInt("Selected Object", &obj);
		ImGui::EndDisabled();

		if (ImGui::InputInt("Camera Number", &cameraIndex, 1))
			SetCameraIndex(cameraIndex);

		ImGui::Checkbox("Draw Gizmos", &drawGizmos);

		float clearCol[3] = { clearColour.r, clearColour.g, clearColour.b, };
		if (ImGui::ColorEdit3("Clear Colour", clearCol))
			Scene::SetClearColour({ clearCol[0], clearCol[1], clearCol[2] });

		if (ImGui::CollapsingHeader("Scene Lighting"))
		{
			float ambientCol[3] = { m_ambientColour.r, m_ambientColour.g, m_ambientColour.b, };
			if (ImGui::ColorEdit3("Ambient Light", ambientCol))
				SetAmbientLightColour({ ambientCol[0], ambientCol[1], ambientCol[2] });

			float sunCol[3] = { m_sunColour.r, m_sunColour.g, m_sunColour.b, };
			if (ImGui::ColorEdit3("Sun Colour", sunCol))
				SetSunColour({ sunCol[0], sunCol[1], sunCol[2] });

			float sunDir[3] = { m_sunDirection.x, m_sunDirection.y, m_sunDirection.z, };
			if (ImGui::SliderFloat3("Sun Direction", &sunDir[0], -1, 1, "%.3f"))
				SetSunDirection({ sunDir[0], sunDir[1], sunDir[2] });

			unsigned int lightsAtStartOfFrame = m_pointLights.size();
			if (lightsAtStartOfFrame == MAX_LIGHTS) ImGui::BeginDisabled();
			if (ImGui::Button("New Point Light"))
				m_pointLights.push_back(Light());
			if (lightsAtStartOfFrame == MAX_LIGHTS) ImGui::EndDisabled();

			// Draw all point lights
			for (int i = 0; i < m_pointLights.size(); i++)
			{
				ImGui::PushID(i);
				float pointCol[3] = { m_pointLights[i].colour.r, m_pointLights[i].colour.g, m_pointLights[i].colour.b, };
				if (ImGui::ColorEdit3("Point Light Colour", &pointCol[0]))
					m_pointLights[i].colour = { pointCol[0], pointCol[1], pointCol[2] };

				float pointPos[3] = { m_pointLights[i].position.x, m_pointLights[i].position.y, m_pointLights[i].position.z, };
				if (ImGui::DragFloat3("Point Light Position", &pointPos[0]))
					m_pointLights[i].position = { pointPos[0], pointPos[1], pointPos[2] };

				ImGui::DragFloat("Intensity", &m_pointLights[i].intensity);
				ImGui::SameLine();
				if (ImGui::Button("Delete"))
				{
					m_pointLights.erase(m_pointLights.begin() + i);
					i--;
				}
				ImGui::PopID();
			}
		}

		if (ImGui::Button("New Object"))
			Scene::CreateObject();

		drawn3DGizmo = false;
		for (auto o : objects)
			o->DrawGUI();

		ImGui::End();

		if (Scene::GetSelectedObject() > 0)
		{
			// Draw Guizmo - very simple implementation - TODO have a 'selected object' context and mousewheel scroll through translate, rotate, scale options - rotate will need to be reworked.
			ImGuizmo::SetRect(0, 0, Window::GetViewPortSize().x, Window::GetViewPortSize().y);
			mat4 view, projection;
			view = Camera::s_instance->GetView();
			projection = Camera::s_instance->GetProjection();
			if (ImGuizmo::Manipulate((float*)&view, (float*)&projection, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, (float*)&s_instance->selectedObject->localTransform))
				s_instance->selectedObject->dirtyTransform = true;
			Scene::s_instance->drawn3DGizmo = true;
		}
	}
}

void Scene::DrawGraphicsGUI()
{
	// Graphics Options - Abstract this as these options develop.
	ImGui::SetNextWindowSize({ 500, 300 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos({ 1700, 300 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Graphics");
	ImGui::BeginDisabled();
	ImGui::InputFloat("Render Time", &renderTime, 0, 0, "%0.6f");
	ImGui::EndDisabled();
	if (ImGui::Checkbox("MSAA Enabled", &MSAAEnabled))
		TextureManager::RefreshFrameBuffers();
	int newMSAASample = MSAASamples;
	if (ImGui::InputInt("MSAA Samples", &newMSAASample))
	{
		if (newMSAASample > MSAASamples)
			MSAASamples *= 2;
		else
			MSAASamples /= 2;

		MSAASamples = glm::clamp(MSAASamples, 2, 16);
		FrameBuffer::SetMSAASampleLevels(MSAASamples);
		TextureManager::RefreshFrameBuffers();
	}

	if (ImGui::Checkbox("SSAO Enabled", &ssao_enabled))
	{
		if (!ssao_enabled)
		{
			for (auto& c : componentCameras)
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
			for (auto& c : componentCameras)
			{
				string ppName = "engine/shader/postProcess/SSAO";
				PostProcess* pp = new PostProcess(c->GetComponentParentObject()->objectName + "_PP_" + ppName);
				pp->SetShader(ShaderManager::GetShaderProgram(ppName));
				pp->SetShaderName(ppName);
				c->m_postProcessStack.push_back(pp);
			}
		}
	}

	ImGui::DragFloat("SSAO Radius", &ssao_radius, 0.01);
	ImGui::SameLine();
	if (ImGui::Button("Reset Radius"))
		ssao_radius = 0.5f;
	ImGui::DragFloat("SSAO Bias", &ssao_bias,0.01);
	ImGui::SameLine();
	if (ImGui::Button("Reset Bias"))
		ssao_bias = 0.025f;

	ImGui::Text("");
	if (ImGui::Button("Reload Shaders"))
		ShaderManager::RecompileAllShaderPrograms();

	ImGui::End();
}

// This will destroy all objects (and their children) marked for deletion.
void Scene::CleanUp()
{
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->CleanUpComponents();
		objects[i]->CleanUpChildren();
		if (objects[i]->markedForDeletion)
		{
			delete objects[i];
			objects.erase(objects.begin() + i);
			i--;
		}
	}
}

// Creates and object and adds it to the parent or scene hierarchy. If you want an object but dont want it added to the heirarchy, then call new Object.
Object* Scene::CreateObject(Object* parent)
{
	Object* o = new Object(s_instance->objectCount++); // NOTE This was objectCount++ in order to increment all object numbers, but honestly we just dont need IDs for everything.. yet?
	if (parent)
	{
		o->parent = parent;
		parent->children.push_back(o);
	}
	else
		s_instance->objects.push_back(o);

	return o;
}
Object* Scene::CreateObject(string name, Object* parent)
{
	Object* o = Scene::CreateObject(parent);
	o->objectName = name;
	return o;
}
Object* Scene::DuplicateObject(Object* object, Object* newParent)
{
	// Copy object properties
	Object* o = CreateObject(newParent);
	o->objectName = object->objectName;
	o->SetLocalPosition(object->localPosition);
	o->SetLocalRotation(object->localRotation);
	o->SetLocalScale(object->localScale);

	// Copy object components and children
	for (auto& component : object->components)
		o->components.push_back(component->Clone(o));

	for (auto& child : object->children)
		Object* c = DuplicateObject(child, o);

	// Refresh
	o->RefreshComponents();
	return o;
}

void Scene::SetClearColour()
{
	glClearColor(s_instance->clearColour.r, s_instance->clearColour.g, s_instance->clearColour.b, 1);
}

void Scene::SetClearColour(vec3 clearColour)
{
	s_instance->clearColour = clearColour;
	glClearColor(clearColour.r, clearColour.g, clearColour.b, 1);
}

vec3 Scene::GetSunColour()
{
	return s_instance->m_sunColour;
}
void Scene::SetSunColour(vec3 sunColour)
{
	s_instance->m_sunColour = sunColour;
}

vec3 Scene::GetSunDirection()
{
	return glm::normalize(s_instance->m_sunDirection);
}
void Scene::SetSunDirection(vec3 sunDirection)
{
	s_instance->m_sunDirection = sunDirection;
}

vec3 Scene::GetAmbientLightColour()
{
	return s_instance->m_ambientColour;
}
void Scene::SetAmbientLightColour(vec3 ambientColour)
{
	s_instance->m_ambientColour = ambientColour;
}

int Scene::GetNumPointLights()
{
	return (int)s_instance->m_pointLights.size();
}

void Scene::SetCameraIndex(int index)
{
	s_instance->cameraIndex = index;
	if (s_instance->cameraIndex > s_instance->cameras.size() - 1)
		s_instance->cameraIndex = s_instance->cameras.size() - 1;
	s_instance->outputCameraFrameBuffer = s_instance->cameras[s_instance->cameraIndex];


	// This is fairly hacky. maybe the cameras should handle setting the audio listener themselves.
	// Perhaps scenee camera and editor camera can become same thing and then audio manager can just look at camera::main or something
	if (s_instance->cameraIndex == 0)
		AudioManager::SetAudioListener(Camera::s_instance->GetAudioListener());
	else
		AudioManager::SetAudioListener(s_instance->componentCameras[s_instance->cameraIndex - 1]->GetAudioListener());
}

void Scene::SetSelectedObject(unsigned int selected)
{
	std::cout << selected << std::endl;
	s_instance->selectedObjectID = selected;
	s_instance->selectedObject = Scene::FindObjectWithID(selected);
}
Object* Scene::FindObjectWithID(unsigned int id)
{
	for (auto o : s_instance->objects)
	{
		if (o->FindObjectWithID(id) != nullptr) return o;
	}

	return nullptr;
}

Object* Scene::FindObjectWithName(string objectName)
{
	for (auto o : s_instance->objects)
		if (o->objectName == objectName) return o;

	for (auto o : s_instance->objects)
		o->FindObjectWithName(objectName);


	return nullptr;
}

void Scene::SaveJSON()
{
	// Create the JSON structures
	ordered_json output;
	ordered_json scene_lighting;
	ordered_json scene_lighting_pointLights;
	ordered_json objectsJSON;

	// meta data
	output["type"] = "scene";
	output["version"] = 1;

	// add lighting
	scene_lighting["clearColour"] = clearColour;
	scene_lighting["ambientColour"] = m_ambientColour;
	scene_lighting["sunColour"] = m_sunColour;
	scene_lighting["sunDirection"] = m_sunDirection;

	// Point lights
	int numPointLights = (int)m_pointLights.size();
	for (int i = 0; i < numPointLights; i++)
		scene_lighting_pointLights.push_back(m_pointLights[i]);

	scene_lighting["pointLights"] = scene_lighting_pointLights;
	output["lighting"] = scene_lighting;


	// Populate objects
	for (int i = 0; i < objects.size(); i++)
		objectsJSON.push_back(*objects[i]);

	output["objects"] = objectsJSON;

	// write to disk
	WriteJSONToDisk(sceneFilename, output);
}

void Scene::LoadJSON(string sceneName)
{
	sceneFilename = sceneName;
	LoadJSON();
}

void Scene::LoadJSON()
{
	// Load the JSON object
	ordered_json input = ReadJSONFromDisk(sceneFilename);
	
	// check version lol
	// load the lighitng data

	ordered_json lighting = input.at("lighting");
	SetClearColour((vec3)lighting["clearColour"]);
	m_ambientColour = lighting.at("ambientColour");
	m_sunColour = lighting.at("sunColour");
	m_sunDirection = lighting.at("sunDirection");

	// point lights
	ordered_json pointLights = lighting.at("pointLights");
	m_pointLights.clear();
	if (pointLights.is_null() == false)
	{
		//m_pointLights.resize(pointLights.size());
		for (auto it = pointLights.begin(); it != pointLights.end(); it++)
		{
			Light light = it.value();
			m_pointLights.push_back(light);
		}
	}

	// clear current objects
	for (auto object = objects.begin(); object != objects.end(); object++)
	{
		auto obj = *object;
		delete obj;
	}
	objects.clear();
	gizmos.clear();

	// read in objects - oooohhh boy here we go.
	ordered_json objectsJSON = input["objects"];

	if (objectsJSON.is_null() == false)
	{
		for (auto it = objectsJSON.begin(); it != objectsJSON.end(); it++)
		{
			auto o = Scene::CreateObject();
			o->LoadFromJSON(it.value());
		}
	}
}

mat4 Scene::GetLightSpaceMatrix()
{
	return s_instance->lightSpaceMatrix;
}

Scene* Scene::s_instance = nullptr;
FrameBuffer* Scene::objectPickBuffer = nullptr;
unordered_map<string, Scene*> Scene::s_instances;

// SSAO Dev
std::vector<glm::vec3> Scene::ssaoKernel;
std::vector<glm::vec3> Scene::ssaoNoise;
Texture* Scene::ssao_noiseTexture = nullptr;

FrameBuffer* Scene::ssao_gBuffer = nullptr;;
FrameBuffer* Scene::ssao_ssaoFBO = nullptr;
FrameBuffer* Scene::ssao_ssaoBlurFBO = nullptr;
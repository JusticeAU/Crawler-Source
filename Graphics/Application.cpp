#include "Application.h"
#include "Graphics.h"

#include "MeshManager.h"
#include "TextureManager.h"
#include "FrameBuffer.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "ModelManager.h"
#include "AudioManager.h"
#include "Scene.h"
#include "SceneEditorCamera.h"
#include "SceneRenderer.h"
#include "Input.h"

#include "MathUtils.h"
#include "FileUtils.h"
#include "LogUtils.h"
#include "Window.h"

#include "DungeonHelpers.h"
#include "DungeonEditor.h"
#include "DungeonPlayer.h"
#include "DungeonGameManager.h"
#include "DungeonArtTester.h"
#include "DungeonMenu.h"

#include "ComponentFactory.h"

Application::Application()
{
	
}

Application::~Application()
{
	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// If we get here the window has closed, perform clean up.
	glfwTerminate();
}

void Application::LaunchArgumentPreLoad(const char* arg)
{
	std::string argument = arg;
	if (argument == "design") developerMode = true;
	else if (argument == "art")
	{
		developerMode = true;
		s_mode = Mode::Art;
	}
	else if (argument == "scene") developerMode = true;
	else if (argument == "dev") developerMode = true;
}

void Application::LaunchArgumentPostLoad(const char* arg)
{
	std::string argument = arg;
		
	if (argument == "design")
	{
		Scene::CreateSceneEditorCamera();
		dungeonEditor = new Crawl::DungeonEditor();
		dungeonEditor->SetDungeon(dungeon);

		s_mode = Mode::Design;
		Scene::ChangeScene("Dungeon");
		dungeonEditor->SetDungeon(dungeon);
		dungeonEditor->Activate();
	}
	else if (argument == "art")
	{
		Scene::CreateSceneEditorCamera();
		Scene* art = Scene::NewScene("CrawlArtTest");
		Scene::s_instance = art;
		art->LoadJSON("crawler/scene/test_art.scene");
		artTester = new Crawl::ArtTester();

		Scene::s_editorCamera->object->SetLocalPosition({ -2.45, -3.75, 2.75 });
		Scene::s_editorCamera->object->SetLocalRotationZ({ -30 });
		Scene::s_editorCamera->object->SetLocalRotationX({ -20 });
		Scene::s_editorCamera->moveSpeed = 5.0f;

		artTester->Activate();
	}
	else if (argument == "scene")
	{
		Scene::CreateSceneEditorCamera();
		s_mode = Mode::Scene;
		Scene::s_instance = Scene::NewScene("New Scene");
		Scene::SetCameraIndex(-1);
	}
	else if (argument == "dev")
	{
		Scene::CreateSceneEditorCamera();
	}
}

void Application::ConstructWindow()
{
	// Initialise GLFW. Make sure it works with an error message.
	LogUtils::Log("Initialising GLFW.");
	if (!glfwInit())
	{
		LogUtils::Log("Failed to initialise GLFW. Closing.");
		return;
	}
	else
		LogUtils::Log("Sucessfully initialised GLFW.");

	// Create GLFWwindow Wrapper (class Window).
	if (developerMode)
	{
		window = new Window(1600, 900, "Crawler");
		//window = new Window(1920, 1080, "Crawler");
	}
	else window = new Window("Crawler");
	
	if (!window->GetGLFWwindow())
	{
		LogUtils::Log("Failed to create GLFW Window. Exiting.");
		glfwTerminate();
		return;
	}
	else
		LogUtils::Log("Sucessfully Created GLFW Window.");

	// Tell GLFW that the window we created is the one we should render to
	LogUtils::Log("Linking GLFW Window to render target.");
	glfwMakeContextCurrent(window->GetGLFWwindow());
	glfwSwapInterval(1); // Enable VSync

	// Tell GLAD to load its OpenGL functions
	LogUtils::Log("Loading GLAD OpenGL functions.");
	if (!gladLoadGL())
	{
		LogUtils::Log("Failed to load GLAD OpenGL Functions. Exiting.");
		return;
	}
	else
		LogUtils::Log("Sucessfully loaded GLAD OpenGL Functions.");

	// Initialise ImGui
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window->GetGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init();

	ImGui::GetIO().IniFilename = NULL; // remove the storage of imgui settings

	// Enable OGL depth testing.
	glEnable(GL_DEPTH_TEST);
	// Enable blending - only used by the dev transparent shader for now
	// Actually not required here, SceneRenderer will now handle this.
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// enable face culling (backface culling is enabled by default).
	glEnable(GL_CULL_FACE);


	SwapBuffers();
}

void Application::LoadResourceManagers()
{
	// Load Assets
	string engineFolder = "engine";
	string gameFolder = "crawler";
	MeshManager::Init();

	TextureManager::Init();
	TextureManager::FindAllFiles(engineFolder);
	TextureManager::FindAllFiles(gameFolder);

	ShaderManager::Init();

	MaterialManager::Init(); // Must be initialised AFTER Texture Manager
	MaterialManager::FindAllFiles(engineFolder);
	MaterialManager::FindAllFiles(gameFolder);

	ModelManager::Init(); // Must be initialised after mesh manager
	ModelManager::LoadAllFiles(engineFolder);
	// CRAWLERCONFIG - Convert from Y up to Z up and scale down by factor of 100
	ModelManager::LoadAllFiles(gameFolder, Crawl::CRAWLER_TRANSFORM);

	ComponentFactory::Init();
	AudioManager::Init();
	AudioManager::LoadAllFiles(engineFolder);
	AudioManager::LoadAllFiles(gameFolder);
	// Load Crawl Game Dependencies
	Scene* scene = Scene::NewScene("Dungeon");
	Scene::s_instance = scene;
	Scene::renderer = new SceneRenderer();
}

void Application::DoLoadingScreen()
{
	if (!developerMode)
	{
		PreloadAssetsAndRenderProgress();
		//TextureManager::PreloadAllFilesContaining("prompt"); // preload the prompt textures because they arent referenced by materials atm.
		//RefreshImGui();
	}
}

void Application::PreloadAssetsAndRenderProgress()
{
	// We could add other systems here and calculate a total
	while (!TextureManager::IsPreloadComplete())
	{
		TextureManager::PreloadNextFile();
		RenderLoadProgress(TextureManager::GetPreloadPercentage());
	}
}

void Application::RenderLoadProgress(float percentageLoaded)
{
	RefreshImGui();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ivec2 vp = Window::GetViewPortSize();
	float barWidth = vp.x - 50;
	float barHeight = (float)vp.y * 0.003f;
	vec2 barPosition = { 25.0f, (float)vp.y * 0.97f };
	int loadingBarTexID = TextureManager::GetTexture("engine/texture/white1x1.tga")->texID;

#ifdef AIE_SHOWCASE
	ImGui::SetNextWindowPos({ barPosition.x, barPosition.y - 50 });
	ImGui::SetNextWindowSize({ barWidth * percentageLoaded, 50 });
	ImGui::Begin("Loading", 0, ImGuiWindowFlags_NoResize);
	ImGui::Text(to_string(percentageLoaded).c_str());
	ImGui::End();
#else
	ImGui::SetNextWindowPos({ barPosition.x, barPosition.y });
	ImGui::SetNextWindowSize({ barWidth, barHeight });
	ImGui::Begin("Loading", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
	ImGui::Image((ImTextureID)loadingBarTexID, { barWidth * percentageLoaded, barHeight });
	ImGui::End();
#endif
	
	RenderImGui();
	SwapBuffers();
	glfwPollEvents(); // if we dont poll events, windows will think the application is hung.
}

void Application::InitialiseAdditionalGameAssets()
{
	dungeon = new Crawl::Dungeon();
	dungeonPlayer = new Crawl::DungeonPlayer();
	menu = new Crawl::DungeonMenu();
	menu->SetPlayer(dungeonPlayer);
	menu->SetApplication(this);
	dungeon->SetPlayer(dungeonPlayer);
	dungeonPlayer->SetDungeon(dungeon);
	dungeonPlayer->SetMenu(menu);
	Crawl::DungeonGameManager::Init();
	Crawl::DungeonGameManager::Get()->SetPlayer(dungeonPlayer);
	Crawl::DungeonGameManager::Get()->SetMenu(menu);
	if(developerMode) Crawl::DungeonGameManager::Get()->manageLobby = false;
}

void Application::InitialiseMenu()
{
	menu->ExecuteReturnToMainMenu();
}

void Application::InitialiseInput()
{
	// Create input system.
	Input::Init(window->GetGLFWwindow());

	// Add support for OSTENT USB Dance Mat
	glfwUpdateGamepadMappings("03000000790000001100000000000000,OSTENT Dance Mat,a:b5,b:b4,back:b8,start:b9,leftshoulder:b6,rightshoulder:b7,dpup:b2,dpleft:b0,dpdown:b1,dpright:b3,platform:Windows,");


	// Add Aliases
	Input::Alias("Forward").RegisterKeyButton(GLFW_KEY_W);
	Input::Alias("Forward").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_UP);
	Input::Alias("Forward").RegisterGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_Y, true);

	Input::Alias("Backward").RegisterKeyButton(GLFW_KEY_S);
	Input::Alias("Backward").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
	Input::Alias("Backward").RegisterGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_Y, false);

	Input::Alias("Left").RegisterKeyButton(GLFW_KEY_A);
	Input::Alias("Left").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_LEFT);
	Input::Alias("Left").RegisterGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_X, true);

	Input::Alias("Right").RegisterKeyButton(GLFW_KEY_D);
	Input::Alias("Right").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);
	Input::Alias("Right").RegisterGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_X, false);

	Input::Alias("TurnLeft").RegisterKeyButton(GLFW_KEY_Q);
	Input::Alias("TurnLeft").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);
	Input::Alias("TurnLeft").RegisterGamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);


	Input::Alias("TurnRight").RegisterKeyButton(GLFW_KEY_E);
	Input::Alias("TurnRight").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
	Input::Alias("TurnRight").RegisterGamepadAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);

	Input::Alias("Interact").RegisterKeyButton(GLFW_KEY_SPACE);
	Input::Alias("Interact").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_A);

	Input::Alias("Wait").RegisterKeyButton(GLFW_KEY_LEFT_ALT);
	Input::Alias("Wait").RegisterKeyButton(GLFW_KEY_LEFT_SHIFT);
	Input::Alias("Wait").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_B);

	Input::Alias("Reset").RegisterKeyButton(GLFW_KEY_R);
	Input::Alias("Reset").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_Y);

	Input::Alias("ResetView").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB);

	Input::Alias("Pause").RegisterGamepadButton(GLFW_GAMEPAD_BUTTON_START);
	Input::Alias("Pause").RegisterKeyButton(GLFW_KEY_ESCAPE);
}

void Application::Run()
{
	// Main Application "Loop"
	LogUtils::Log("Starting Main Loop.");
	float currentTime = glfwGetTime();
	while (!glfwWindowShouldClose(window->GetGLFWwindow()))
	{
		float newTime = glfwGetTime();
		float deltaTime = newTime - currentTime;
		currentTime = newTime;

		RefreshImGui();

		// Clear the screen buffer and the depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update my application here.
		Update(deltaTime);

		RenderImGui();

		SwapBuffers();

		// Tell GLFW to check if anything is going on with input
		glfwPollEvents();
		float currentTime = glfwGetTime();
	}
}

void Application::RefreshImGui()
{
	// Refresh ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void Application::RenderImGui()
{
	// Render Imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::SwapBuffers()
{
	// Swap the 'working' buffer to the 'live' buffer - this means the frame is over!
	glfwSwapBuffers(window->GetGLFWwindow());
}

void Application::Update(float delta)
{
	switch (s_mode)
	{
	case Mode::Menu:
	{
		menu->Update(delta);
		break;
	}
	case Mode::Game:
	{
		if (dungeonPlayer->Update(delta))
			dungeon->Update();

		Crawl::DungeonGameManager::Get()->Update(delta);

		dungeon->UpdateVisuals(delta);
		break;
	}
	case Mode::Design:
	{
		Scene::s_editorCamera->Update(delta);
		Scene::s_editorCamera->DrawGUI();
		dungeonEditor->Update();
		dungeonEditor->DrawGUI();
		
		dungeon->UpdateVisuals(delta);
		break;
	}
	case Mode::Art:
	{
		if (Scene::GetCameraIndex() == -1)
		{
			Scene::s_editorCamera->Update(delta);
			Scene::s_editorCamera->DrawGUI();
		}
		artTester->Update(delta);
		artTester->DrawGUI();
		MaterialManager::DrawGUI();
		break;
	}
	case Mode::Programming:
	{
		MeshManager::DrawGUI();
		TextureManager::DrawGUI();
		ShaderManager::DrawGUI();
		MaterialManager::DrawGUI();
		ModelManager::DrawGUI();
		AudioManager::DrawGUI();
		Scene::s_instance->DrawGUI();
		Scene::s_instance->renderer->DrawGUI();
		//Scene::s_instance->renderer->DrawShadowCubeMappingGUI();
		Scene::s_editorCamera->Update(delta);
		Scene::s_editorCamera->DrawGUI();
		break;
	}
	case Mode::Scene:
	{
		
	}

	}

	Input::Update();

	if (developerMode)
	{
		if (dungeonEditor != nullptr && dungeonEditor->requestedGameMode)
		{
			s_mode = Mode::Game;
			Scene::ChangeScene("Dungeon");
			Scene::SetCameraByName("Player Camera");
			dungeonEditor->requestedGameMode = false;
		}

		if (dungeonEditor != nullptr && s_mode == Mode::Game && Input::Keyboard(GLFW_KEY_ESCAPE).Down())
		{
			s_mode = Mode::Design;
			dungeonEditor->Activate();
		}

		if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Pressed() && Input::Keyboard(GLFW_KEY_F11).Down())
		{
			if (s_mode == Mode::Programming)
				s_mode = s_modeOld;
			else
			{
				s_modeOld = s_mode;
				s_mode = Mode::Programming;
			}
		}
	}


	AudioManager::s_instance->Update(delta);

	Scene::s_instance->Update(delta);
	Scene::s_instance->CleanUp();

	Scene::s_instance->Render();
}

Application::Mode Application::s_mode = Mode::Game;
Application::Mode Application::s_modeOld = Mode::Game;

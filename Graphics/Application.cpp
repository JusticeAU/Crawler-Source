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
#include "Input.h"
#include "Camera.h"

#include "MathUtils.h"
#include "FileUtils.h"
#include "LogUtils.h"
#include "Window.h"

#include "DungeonHelpers.h"
#include "DungeonEditor.h"
#include "DungeonPlayer.h"
#include "DungeonArtTester.h"

#include "ComponentFactory.h"

Application::Application()
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
	window = new Window(1600, 900, "Crawler", nullptr);
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
	//glfwSwapInterval(0); // Disable vsync

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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// enable face culling (backface culling is enabled by default).
	glEnable(GL_CULL_FACE);

	// Create our 'Editor' Camera
	float aspect;
	int width, height;
	glfwGetWindowSize(window->GetGLFWwindow(), &width, &height);
	aspect = width / (float)height;
	camera = new Camera(aspect, "Main Camera");

	// Create input system.
	Input::Init(window->GetGLFWwindow());

	// Load Assets
	string engineFolder = "engine";
	string gameFolder = "crawler";
	MeshManager::Init();
	
	TextureManager::Init();
	TextureManager::FindAllFiles(engineFolder);
	TextureManager::FindAllFiles(gameFolder);
	// Add the scene camera to our framebuffer here
	// This was originally in Scene constructor, but moving to scene instances caused this to conflict per scene.
	TextureManager::s_instance->AddFrameBuffer(Camera::s_instance->name.c_str(), Camera::s_instance->GetFrameBuffer());
	TextureManager::s_instance->AddFrameBuffer((Camera::s_instance->name + "Blit").c_str(), Camera::s_instance->GetFrameBufferBlit());
	TextureManager::s_instance->AddFrameBuffer((Camera::s_instance->name + "Processed").c_str(), Camera::s_instance->GetFrameBufferProcessed());



	ShaderManager::Init();
	
	MaterialManager::Init(); // Must be initialised AFTER Texture Manager
	MaterialManager::FindAllFiles(engineFolder);
	MaterialManager::FindAllFiles(gameFolder);
	
	ModelManager::Init(); // Must be initialised after mesh manager
	ModelManager::LoadAllFiles(engineFolder);
	// CRAWLERCONFIG - Convert from Y up to Z up and scale down by factor of 100
	ModelManager::LoadAllFiles(gameFolder, Crawl::CRAWLER_TRANSFORM);

	//Scene::Init();
	ComponentFactory::Init();
	AudioManager::Init();
	AudioManager::LoadAllFiles(engineFolder);
	AudioManager::LoadAllFiles(gameFolder);

	AudioManager::SetAudioListener(camera->GetAudioListener());

	// Load Crawl Game Dependencies
	Scene* scene = Scene::NewScene("Dungeon");
	Scene::s_instance = scene;

	dungeon = new Crawl::Dungeon();
	dungeonPlayer = new Crawl::DungeonPlayer();
	dungeon->SetPlayer(dungeonPlayer);
	dungeonPlayer->SetDungeon(dungeon);


	//SSAO Dev
	TextureManager::SetPreviewTexture("framebuffers/SSAOBlur");
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

void Application::LaunchArgument(char* arg)
{
	std::string argument = arg;
	if (argument == "design" || argument == "art" || argument == "dev")
	{
		developerMode = true;

		
		if (argument == "design")
		{
			dungeonEditor = new Crawl::DungeonEditor();
			dungeonEditor->SetDungeon(dungeon);

			s_mode = Mode::Design;
			Scene::ChangeScene("Dungeon");
			dungeonEditor->SetDungeon(dungeon);
			dungeonEditor->Activate();
			Window::SetWindowTitle("Crawler Editor");
		}
		else if (argument == "art")
		{
			Scene* art = Scene::NewScene("CrawlArtTest");
			Scene::s_instance = art;
			art->LoadJSON("crawler/scene/test_art.scene");
			artTester = new Crawl::ArtTester();

			s_mode = Mode::Art;
			artTester->Activate();
		}
	}
}

void Application::InitGame()
{
	MaterialManager::PreloadAllFiles();
	dungeon->Load("crawler/dungeon/start.dungeon");
	Scene::SetCameraIndex(1);
	/*if (!developerMode)
		dungeonPlayer->Respawn();*/
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

		// Refresh ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();


		// Clear the screen buffer and the depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update my application here.
		Update(deltaTime);

		// Render Imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap the 'working' buffer to the 'live' buffer - this means the frame is over!
		glfwSwapBuffers(window->GetGLFWwindow());

		// Tell GLFW to check if anything is going on with input
		glfwPollEvents();
		float currentTime = glfwGetTime();
	}
}

void Application::Update(float delta)
{
	//TextureManager::DrawTexturePreview();
	switch (s_mode)
	{
	case Mode::Game:
	{
		if(dungeonPlayer->Update(delta))
			dungeon->Update();

		dungeon->UpdateVisuals(delta);
		break;
	}
	case Mode::Design:
	{
		camera->DrawGUI();
		camera->Update(delta);
		dungeonEditor->Update();
		dungeonEditor->DrawGUI();
		break;
	}
	case Mode::Art:
	{
		artTester->Update(delta);
		artTester->DrawGUI();

		if (Scene::GetCameraIndex() == 0)
		{
			camera->DrawGUI();
			camera->Update(delta);
		}
		break;
	}
	case Mode::Programming:
	{
		camera->DrawGUI();
		camera->Update(delta);
		MeshManager::DrawGUI();
		TextureManager::DrawGUI();
		ShaderManager::DrawGUI();
		MaterialManager::DrawGUI();
		ModelManager::DrawGUI();
		AudioManager::DrawGUI();
		Scene::s_instance->DrawGUI();
		Scene::s_instance->DrawGraphicsGUI();
		break;
	}

	}

	Input::Update();

	if (developerMode)
	{
		if (dungeonEditor != nullptr && dungeonEditor->requestedGameMode)
		{
			s_mode = Mode::Game;
			Scene::ChangeScene("Dungeon");
			Scene::SetCameraIndex(1);
			dungeonEditor->requestedGameMode = false;
		}

		if (dungeonEditor != nullptr && s_mode == Mode::Game && Input::Keyboard(GLFW_KEY_ESCAPE).Down())
		{
			s_mode = Mode::Design;
			dungeonEditor->Activate();
		}

		if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Pressed() && Input::Keyboard(GLFW_KEY_F12).Down())
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


	AudioManager::s_instance->Update();

	Scene::s_instance->Update(delta);
	Scene::s_instance->Render();
	Scene::s_instance->DrawCameraToBackBuffer();
	Scene::s_instance->CleanUp();
}

Application::Mode Application::s_mode = Mode::Game;
Application::Mode Application::s_modeOld = Mode::Game;

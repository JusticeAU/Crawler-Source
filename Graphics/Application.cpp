#include "Application.h"
#include "Graphics.h"

#include "MeshManager.h"
#include "TextureManager.h"
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

#include "DungeonEditor.h"
#include "DungeonPlayer.h"
#include "ArtTester.h"

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

	// Load Assets
	string engineFolder = "engine";
	string gameFolder = "crawler";
	MeshManager::Init();
	TextureManager::Init();
	TextureManager::LoadAllFiles(engineFolder);
	TextureManager::LoadAllFiles(gameFolder);
	ShaderManager::Init();
	MaterialManager::Init(); // Must be initialised AFTER Texture Manager
	MaterialManager::LoadAllFiles(engineFolder);
	MaterialManager::LoadAllFiles(gameFolder);
	ModelManager::Init(); // Must be initialised after mesh manager
	ModelManager::LoadAllFiles(engineFolder);
	ModelManager::LoadAllFiles(gameFolder);
	//Scene::Init();
	ComponentFactory::Init();
	AudioManager::Init();
	AudioManager::SetAudioListener(camera->GetAudioListener());
	
	// Create input system.
	Input::Init(window->GetGLFWwindow());

	// Load Crawl Game Dependencies
	Scene* scene = Scene::NewScene("Dungeon");
	Scene::s_instance = scene;
	scene->LoadJSON("crawler/scene/dungeon.scene");

	dungeon = new Crawl::Dungeon();
	dungeonPlayer = new Crawl::DungeonPlayer();
	dungeonPlayer->SetDungeon(dungeon);
	
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
	if (argument == "design" || argument == "art")
	{
		developerMode = true;

		dungeonEditor = new Crawl::DungeonEditor();
		dungeonEditor->SetDungeon(dungeon);

		artTester = new Crawl::ArtTester();
		Scene* art = Scene::NewScene("CrawlArtTest");
		Scene::s_instance = art;
		art->LoadJSON("crawler/scene/test_art.scene");
		
		if (argument == "design")
		{
			s_mode = Mode::Design;
			Scene::ChangeScene("Dungeon");
			dungeonEditor->SetDungeon(dungeon);
			dungeonEditor->Activate();
		}
		else if (argument == "art")
		{
			s_mode = Mode::Art;
			artTester->Activate();
		}
	}

}

void Application::InitGame()
{
	dungeon->Load("crawler/dungeon/test level.dungeon");
	Scene::SetCameraIndex(1);
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
	switch (s_mode)
	{
	case Mode::Game:
	{
		dungeonPlayer->Update(delta);
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
		artTester->DrawGUI();
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
		break;
	}

	}

	Input::Update();

	if (developerMode)
	{
		if (Input::Keyboard(GLFW_KEY_1).Down())
		{
			s_mode = Mode::Game;
			Scene::ChangeScene("Dungeon");
			Scene::SetCameraIndex(1);

		}
		if (Input::Keyboard(GLFW_KEY_2).Down())
		{
			s_mode = Mode::Design;
			dungeonEditor->Activate();
		}
		if (Input::Keyboard(GLFW_KEY_3).Down())
		{
			s_mode = Mode::Art;
			artTester->Activate();
		}
		if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Pressed() && Input::Keyboard(GLFW_KEY_F12).Down())
			s_mode = Mode::Programming;
	}


	AudioManager::s_instance->Update();

	Scene::s_instance->Update(delta);
	Scene::s_instance->Render();
	Scene::s_instance->DrawGizmos();
	Scene::s_instance->DrawCameraToBackBuffer();
	Scene::s_instance->CleanUp();
}

Application::Mode Application::s_mode = Mode::Game;
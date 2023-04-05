#include "Application.h"
#include "Graphics.h"

#include "MeshManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "ModelManager.h"
#include "Scene.h"
#include "Input.h"
#include "Camera.h"

#include "MathUtils.h"
#include "FileUtils.h"
#include "LogUtils.h"

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

	// Set resolution and window name.
	LogUtils::Log("Creating GLFW Window.");
	window = glfwCreateWindow(1600, 900, "Graffix", nullptr, nullptr);
	if (!window)
	{
		LogUtils::Log("Failed to create GLFW Window. Exiting.");
		glfwTerminate();
		return;
	}
	else
		LogUtils::Log("Sucessfully Created GLFW Window.");

	// Tell GLFW that the window we created is the one we should render to
	LogUtils::Log("Linking GLFW Window to render target.");
	glfwMakeContextCurrent(window);
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
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Enable OGL depth testing.
	glEnable(GL_DEPTH_TEST);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Load Assets
	MeshManager::Init();
	TextureManager::Init();
	ShaderManager::Init();
	MaterialManager::Init(); // Must be initialised AFTER Texture Manager
	ModelManager::Init(); // Must be initialised after mesh manager
	Scene::Init();
	
	// Create input system.
	Input::Init(window);
	
	// Create our Camera
	float aspect;
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	aspect = width / (float)height;
	camera = new Camera(aspect,  window);

	// Create a default object
	Scene::CreateObject("Default Object");
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

void Application::Run()
{
	// Main Application "Loop"
	LogUtils::Log("Starting Main Loop.");
	float currentTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		float newTime = glfwGetTime();
		float deltaTime = newTime - currentTime;
		currentTime = newTime;

		// Refresh ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Clear the screen buffer and the depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update my application here.
		Update(deltaTime);

		// Render Imgui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap the 'working' buffer to the 'live' buffer - this means the frame is over!
		glfwSwapBuffers(window);

		// Tell GLFW to check if anything is going on with input
		glfwPollEvents();
		float currentTime = glfwGetTime();
	}
}

void Application::Update(float delta)
{
	MeshManager::DrawGUI();
	TextureManager::DrawGUI();
	ShaderManager::DrawGUI();
	MaterialManager::DrawGUI();
	ModelManager::DrawGUI();
	camera->DrawGUI();

	Input::Update();
	camera->Update(delta);

	Scene::s_instance->Update(delta);
	Scene::s_instance->DrawObjects();
	Scene::s_instance->DrawGUI();
	Scene::s_instance->CleanUp();
}

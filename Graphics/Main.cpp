// These includes are specif to the way we've set up GLFW and GLAD.
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glad.h"
#include "glm.hpp"
#include "MathUtils.h"
#include "FileUtils.h"
#include <iostream>
#include "TestApplication.h"

int main(void)
{
	GLFWwindow* window;

	// Initialise GLFW. Make sure it works with an error message.
	std::cout << "Initialising GLFW." << std::endl;
	if (!glfwInit())
	{
		std::cout << "Failed to initialise GLFW. Closing." << std::endl;
		return -1;
	}
	else
		std::cout << "Sucessfully initialised GLFW." << std::endl;

	// Set resolution and window name.
	std::cout << "Creating GLFW Window." << std::endl;
	window = glfwCreateWindow(1280, 720, "Graffix", nullptr, nullptr);
	if (!window)
	{
		std::cout << "Failed to create GLFW Window. Exiting." << std::endl;
		glfwTerminate();
		return -1;
	}
	else
		std::cout << "Sucessfully Created GLFW Window." << std::endl;

	// Tell GLFW that the window we created is the one we should render to
	std::cout << "Linking GLFW Window to render target." << std::endl;
	glfwMakeContextCurrent(window);

	// Tell GLAD to load its OpenGL functions
	std::cout << "Loading GLAD OpenGL functions." << std::endl;
	if (!gladLoadGL())
	{
		std::cout << "Failed to load GLAD OpenGL Functions. Exiting." << std::endl;
		return -1;
	}
	else
		std::cout << "Sucessfully loaded GLAD OpenGL Functions." << std::endl;

	// Some init stuff ehehe
	glClearColor(0.8f, 0.0f, 0.0f, 1.0f);
	
	float currentTime = glfwGetTime();

	
	// Construct my actual application
	std::cout << "Constructing Application." << std::endl;
	Application* app = new TestApplication();
	
	std::cout << "Starting Main Loop." << std::endl;
	// Main Application "Loop"
	while (!glfwWindowShouldClose(window))
	{
		float newTime = glfwGetTime();
		float deltaTime = newTime - currentTime;
		currentTime = newTime;
		// Clear the screen - eventually do rendering here
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Update my application here.
		app->Update(deltaTime);

		// Swap the buggers - this means the frame is over!
		glfwSwapBuffers(window);

		// Tell GLFW to check if anything is going on with input
		glfwPollEvents();
		float currentTime = glfwGetTime();
	}

	// If we get here the window has closed, perform clean up.
	glfwTerminate();
}


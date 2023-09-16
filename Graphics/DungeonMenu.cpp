#include "DungeonMenu.h"
#include "graphics.h"
#include "Application.h"

Crawl::DungeonMenu::DungeonMenu()
{
	window = Window::Get();
	glm::ivec2 size = window->GetViewPortSize();
	halfWidth = size.x / 2;
	halfHeight = size.y / 2;
}

void Crawl::DungeonMenu::DrawMenu()
{
	ImGui::SetNextWindowSize({ (float)titleMenuSize.x, (float)titleMenuSize.y });
	ImGui::SetNextWindowPos({ (float)halfWidth, (float)halfHeight }, 0, { 0.5f, 0.5f });
	ImGui::Begin("Crawler", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	if (ImGui::Button(textNewGame.c_str()))
	{
		ExecuteMenuOption(1);
		// do stuff
	}
	if (ImGui::Button(textQuitGame.c_str()))
	{
		ExecuteMenuOption(2);
		// do stuff
	}


	ImGui::End();
}

void Crawl::DungeonMenu::ExecuteMenuOption(int optionID)
{
	switch (optionID)
	{
	case 1:
	{
		app->InitGame();
		app->s_mode = Application::Mode::Game;
		break;
		// stuff
	}
	case 2:
	{
		glfwSetWindowShouldClose(window->GetGLFWwindow(), GLFW_TRUE);
		break;
		// stuff
	}
	default:
	{
		// Otherwise
		break;
	}
	}
}

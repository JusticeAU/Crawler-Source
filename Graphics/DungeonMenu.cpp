#include "DungeonMenu.h"
#include "DungeonPlayer.h"
#include "graphics.h"
#include "Application.h"

Crawl::DungeonMenu::DungeonMenu()
{

}

void Crawl::DungeonMenu::DrawMainMenu()
{
	UpdatePositions();

	ImGui::SetNextWindowSize({ (float)titleMenuSize.x, (float)titleMenuSize.y });
	ImGui::SetNextWindowPos({ (float)halfWidth, (float)halfHeight }, 0, { 0.5f, 0.5f });
	ImGui::Begin("Crawler", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	if (ImGui::Button(textNewGame.c_str()))
	{
		ExecuteNewGame();
		// do stuff
	}
	if (ImGui::Button(textToggleFullScreen.c_str()))
	{
		ExecuteToggleFullScreen();
		// do stuff
	}
	if (ImGui::Button(textQuitGame.c_str()))
	{
		ExecuteQuitGame();
		// do stuff
	}


	ImGui::End();
}

void Crawl::DungeonMenu::DrawPauseMenu()
{
	UpdatePositions();

	ImGui::SetNextWindowSize({ (float)titleMenuSize.x, (float)titleMenuSize.y+50 });
	ImGui::SetNextWindowPos({ (float)halfWidth, (float)halfHeight }, 0, { 0.5f, 0.5f });
	ImGui::Begin("Crawler", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	
	if (ImGui::Button(textResumeGame.c_str()))			ExecuteResumeGame();
	if (lobbyReturnEnabled)
	{
		if (ImGui::Button(textReturnToLobby.c_str()))	ExecuteReturnToLobby();
	}
	if (ImGui::Button(textToggleFullScreen.c_str()))	ExecuteToggleFullScreen();
	if (ImGui::Button(textQuitGame.c_str()))			ExecuteQuitGame();


	ImGui::End();
}

void Crawl::DungeonMenu::UpdatePositions()
{
	window = Window::Get();
	glm::ivec2 size = window->GetViewPortSize();
	halfWidth = size.x / 2;
	halfHeight = size.y / 2;
}

void Crawl::DungeonMenu::ExecuteNewGame()
{
	app->InitGame();
	app->s_mode = Application::Mode::Game;
}

void Crawl::DungeonMenu::ExecuteQuitGame()
{
	glfwSetWindowShouldClose(window->GetGLFWwindow(), GLFW_TRUE);
}

void Crawl::DungeonMenu::ExecuteResumeGame()
{
	player->SetStateIdle();;
}

void Crawl::DungeonMenu::ExecuteToggleFullScreen()
{
	Window::GetWindow()->ToggleFullscreen();
}

void Crawl::DungeonMenu::ExecuteReturnToLobby()
{
	player->SetStateIdle();
	player->ReturnToLobby();
}

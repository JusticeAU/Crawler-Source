#pragma once
#include <string>
#include "Window.h"

using std::string;

class Application;

/// <summary>
/// DungeonMeny is the base class for handling all game meny operations.
/// Right now it's basicly at a MVP level, providing some base functions and an IMGUI draw to test those functions are working.
/// We'll configure this to be a main menu that appears before the game starts, and have some configuration options like
/// graphical settings, accessability etc.
/// It will also move to talk to the game manager once that is created - Instead of a bunch of functionality living on the player class.
/// </summary>
namespace Crawl
{
	class DungeonPlayer;
	class DungeonMenu
	{
	public:
		DungeonMenu();
		void DrawMainMenu();
		void DrawPauseMenu();

		void UpdatePositions();

		void SetApplication(Application* app) { this->app = app; }
		void SetPlayer(DungeonPlayer* player) { this->player = player; }
		void SetLobbyReturnEnabled() { lobbyReturnEnabled = true; }

		void ExecuteNewGame();
		void ExecuteQuitGame();
		void ExecuteResumeGame();
		void ExecuteToggleFullScreen();
		void ExecuteReturnToLobby();
		void ExecuteRestartGame();

		string textNewGame = "New Game";
		string textQuitGame = "Quit Game";
		string textResumeGame = "Resume Game";
		string textToggleFullScreen = "Toggle FullScreen";
		string textReturnToLobby = "Return to Lobby";
		string textRestartGame = "Restart Game";

	private:
		glm::ivec2 titleMenuSize = { 300, 225 };
		int halfWidth;
		int halfHeight;
		Window* window = nullptr;
		Application* app = nullptr;
		DungeonPlayer* player = nullptr;

		bool lobbyReturnEnabled = false;
	};
}


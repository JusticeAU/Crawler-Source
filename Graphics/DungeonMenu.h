#pragma once
#include <string>
#include "Window.h"

using std::string;

class Application;

namespace Crawl
{
	class DungeonMenu
	{
	public:
		DungeonMenu();
		void DrawMenu();

		void SetApplication(Application* app) { this->app = app; }

		void ExecuteMenuOption(int optionID);

		string textNewGame = "New Game";
		string textQuitGame = "Quit Game";

	private:
		glm::ivec2 titleMenuSize = { 100, 75 };
		int halfWidth;
		int halfHeight;
		Window* window = nullptr;
		Application* app = nullptr;

	};
}


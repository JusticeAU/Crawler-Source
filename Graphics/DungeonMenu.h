#pragma once
#include <string>
#include "Window.h"

using std::string;

class Application;
class Texture;
class Object;

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
	class DungeonMenuButton;
	class DungeonMenu
	{
	public:
		enum class Menu
		{
			None,
			Main,
			Pause,
			Thanks,
			Credits
		};
		enum class IntroStage
		{
			Idle,
			Turn,
			Fly,
			Text
		};
		DungeonMenu();

		void OpenMenu(Menu menu);

		void Update(float delta);
		void DrawMainMenu(float delta);
		void DrawPauseMenu(float delta);
		void DrawCredits(float delta);
		void DrawThanks(float delta);

		void DrawBlackScreen(float alpha, bool onTop = false);
		void DrawImage(Texture* tex, float alpha);

		void UpdatePositions();

		void UpdateMainMenuCamera(float delta);

		void SetApplication(Application* app) { this->app = app; }
		void SetPlayer(DungeonPlayer* player) { this->player = player; }
		void SetLobbyReturnEnabled() { lobbyReturnEnabled = true; }
		void SetMenuCameraObject(Object* cameraObject) { this->cameraObject = cameraObject; }

		void ExecuteNewGameButton();
		void StartNewGame();
		void ExecuteQuitGame();
		void ExecuteResumeGame();
		void ExecuteToggleFullScreen();
		void ExecuteReturnToLobby();
		void ExecuteReturnToMainMenu();


		string textToggleFullScreen = "Toggle FullScreen";

		// Menu Texture Paths
		string menuTitleCardTexPath = "crawler/texture/gui/menu/title.tga";

		string menuNewGameTexPath = "crawler/texture/gui/menu/new.tga";
		string menuSettingsTexPath = "crawler/texture/gui/menu/settings.tga";
		string menuQuitTexPath = "crawler/texture/gui/menu/quit.tga";
		
		string menuResumeGameTexPath = "crawler/texture/gui/menu/resume.tga";
		string menuReturnToLobbyPath = "crawler/texture/gui/menu/returnToLobby.tga";
		string menuReturnToMenuTexPath = "crawler/texture/gui/menu/returnToMenu.tga";


		// Main
		DungeonMenuButton* buttonNewGame;
		DungeonMenuButton* buttonSettings;
		DungeonMenuButton* buttonQuit;

		// Pause
		DungeonMenuButton* pauseButtonResumeGame;
		DungeonMenuButton* pauseButtonReturnLobby;
		DungeonMenuButton* pauseButtonReturnToMenu;
		DungeonMenuButton* pauseButtonQuit;

		// Thanks
		string menuThanksCardTexPath = "crawler/texture/gui/prompt_thankyou.tga";
		Texture* menuThanksCardTex = nullptr;


		// Main Menu camera
		Object* cameraObject = nullptr;
		IntroStage introStage = IntroStage::Idle;
		bool newGameSequenceStarted = false;
		float newGameSequenceTime = 0.0f;
		string blackTexPath = "engine/texture/black1x1.tga";
		Texture* blackTex = nullptr;

		Texture* intro01 = nullptr;
		Texture* intro02 = nullptr;
		Texture* intro03 = nullptr;
		Texture* intro04 = nullptr;
		Texture* intro05 = nullptr;
		Texture* introPressSpace = nullptr;


	private:
		Menu currentMenu = Menu::None;

		glm::vec2 screenSize = { 0,0 };
		glm::ivec2 titleMenuSize = { 600, 300 };
		glm::ivec2 pauseMenuSize = { 600, 600 };

		int mainMenuXOffset = 0.0f;
		int mainMenuYOffset = 0.0f;
		Window* window = nullptr;
		Application* app = nullptr;
		DungeonPlayer* player = nullptr;

		bool lobbyReturnEnabled = false;
		bool indent = false;

		float blackScreenFraction = 1.0f;

		float noiseAccumulator = 0.0f;
		glm::vec3 menuCameraPositionMax = { 0.03f, 0.02f, 0.03f };
		glm::vec3 menuCameraPositionFrequency = { 0.05f, 0.05f, 0.03f };
		glm::vec3 menuCameraPositionAccumulatorOffset = { 0.0f, 50.0f, 100.0f };

		glm::vec3 menuCameraRotationMax = { 0.0f, 0.0f, 0.0f };
		glm::vec3 menuCameraRotationFrequency = { 1.0f, 1.0f, 1.0f };
		glm::vec3 menuCameraRotationAccumulatorOffset = { 150.0f, 200.0f, 250.0f };

		// Audio
		string musicFolder = "crawler/sound/stream/";
		string sfxFolder = "crawler/sound/load/";

		string menuMusic = musicFolder + "main_menu.ogg";

	};
}
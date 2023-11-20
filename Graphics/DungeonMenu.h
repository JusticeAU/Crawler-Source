#pragma once
#include "Window.h"

#include <stack>
#include <vector>
#include <string>


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
	class DungeonEditor;
	class DungeonMenuButton;
	class DungeonMenu
	{
	public:
		enum class Menu
		{
			None,
			Main,
			Settings,
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
		int selectedMenuOption = 0;
		std::stack<Menu> menuStack;
		std::vector<DungeonMenuButton*> menuButtonsMain;
		std::vector<DungeonMenuButton*> menuButtonsSettings;
		std::vector<DungeonMenuButton*> menuButtonsCredits;
		std::vector<DungeonMenuButton*> menuButtonsPause;
		std::vector<DungeonMenuButton*> menuButtonsThanks;

		void Initialise();

		void OpenMenu(Menu menu);

		void Update(float delta);
		void UpdateInput(std::vector<DungeonMenuButton*>* menuButtons);
		void DrawMainMenu(float delta);
		void DrawTitleCard(float alpha);
		void DrawSettingsMenu(float delta);
		void DrawPauseMenu(float delta);
		void DrawCredits(float delta);
		void DrawThanks(float delta);

		void DrawBlackScreen(float alpha, bool onTop = false);
		static void DrawImage(Texture* tex, float alpha);

		void UpdatePositions();

		void UpdateMainMenuCamera(float delta);

		void SetApplication(Application* app) { this->app = app; }
		void SetPlayer(DungeonPlayer* player) { this->player = player; }
		void SetEditor(DungeonEditor* editor) { this->editor = editor; }
		void SetLobbyReturnEnabled() { lobbyReturnEnabled = true; }
		void SetMenuCameraObject(Object* cameraObject) { this->cameraObject = cameraObject; }

		void ExecuteNewGameButton();
		void StartNewGame();
		void ExecuteLevelEditorButton();
		void ExecuteCreditsButton();
		void ExecuteQuitGameButton();
		void ExecuteBackButton();
		void ExecuteResumeGameButton();
		void ExecuteSettingsButtonViaPause();
		void ExecuteSettingsButton();
		void ExecuteSettingsInvertY();
		void ExecuteSettingsAlwaysFreelook();
		void ExecuteSettingsToggleCrosshair();
		void ExecuteSettingsVolumeUp();
		void ExecuteSettingsVolumeDown();
		void ExecuteToggleFullScreen();
		void ExecuteReturnToLobbyButton();
		void ExecuteReturnToMainMenuButton();


		string textToggleFullScreen = "Toggle FullScreen";

		// Menu Texture Paths
		string menuTitleCardTexPath = "crawler/texture/gui/menu/title.tga";
		glm::ivec2 menuTitleCardPosition = { 0,0 };
		Texture* menuTitleCardTexture = nullptr;
		float menuTitleCardAlpha = 1.0f;

		string menuNewGameTexPath = "crawler/texture/gui/menu/new.tga";
		string menuEditorTexPath = "crawler/texture/gui/menu/editor.tga";
		string menuSettingsTexPath = "crawler/texture/gui/menu/settings.tga";
		string menuCreditsTexPath = "crawler/texture/gui/menu/credits.tga";
		string menuQuitTexPath = "crawler/texture/gui/menu/quit.tga";

		// Settings
		string menuSettingsFreelookTexPath = "crawler/texture/gui/menu/settings_alwaysfreelook.tga";
		string menuSettingsFullscreenTexPath = "crawler/texture/gui/menu/settings_fullscreen.tga";
		string menuSettingsInvertTexPath = "crawler/texture/gui/menu/settings_invert.tga";
		string menuSettingsCrosshairTexPath = "crawler/texture/gui/menu/settings_crosshair.tga";

		//string menuSettingsVolumeTexPath = "crawler/texture/gui/menu/settings_volume.tga";
		bool settingsMenuViaPause = false;

		// Credits
		string menuCreditsBigTexPath = "crawler/texture/gui/menu/creditsTeam.tga";
		Texture* menuCreditsTexture = nullptr;
		
		// Pause
		string menuResumeGameTexPath = "crawler/texture/gui/menu/resume.tga";
		string menuReturnToLobbyPath = "crawler/texture/gui/menu/returnToLobby.tga";
		string menuReturnToMenuTexPath = "crawler/texture/gui/menu/returnToMenu.tga";


		// Thanks
		string menuThanksCardTexPath = "crawler/texture/gui/prompt_thankyou.tga";
		Texture* menuThanksCardTex = nullptr;

		string menuBackTexPath = "crawler/texture/gui/menu/back.tga";


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
		Texture* introPressSpacePad = nullptr;


	private:
		glm::vec2 screenSize = { 0,0 };
		glm::vec2 titleMenuSize = { 600, 500 };
		glm::vec2 creditsMenuSize = { 600, 150 };
		glm::vec2 settingsMenuSize = { 700, 500 };
		glm::vec2 pauseMenuSize = { 600, 600 };
		glm::vec2 thanksMenuSize = { 600, 200 };
		glm::vec2 creditsPosition = { 0,0 };

		float mainMenuXOffset = 0.0f;
		float mainMenuYOffset = 0.0f;
		Window* window = nullptr;
		Application* app = nullptr;
		DungeonPlayer* player = nullptr;
		DungeonEditor* editor = nullptr;
		bool mouseHasBeenReleased = true; // Quick hack for how we're bypassing input manager controls due to ImGui usage

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
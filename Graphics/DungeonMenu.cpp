#include "DungeonMenu.h"
#include "DungeonMenuButton.h"
#include "DungeonPlayer.h"
#include "DungeonGameManager.h"
#include "graphics.h"
#include "Application.h"
#include "Input.h"
#include "Scene.h"
#include "Object.h"

#include "gtc/noise.hpp"
#include "MathUtils.h"
#include "gtx/easing.hpp"
#include "TextureManager.h"
#include "AudioManager.h"
#include "LogUtils.h"

Crawl::DungeonMenu::DungeonMenu()
{
	// Populate Main Menu
	DungeonMenuButton* button = new DungeonMenuButton("New Game", menuNewGameTexPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteNewGameButton, this);
	menuButtonsMain.push_back(button);

	button = new DungeonMenuButton("Settings", menuSettingsTexPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteNewGameButton, this);
	//menuButtons[(int)Menu::Main].push_back(button);

	button = new DungeonMenuButton("QuitGame", menuQuitTexPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteQuitGame, this);
	menuButtonsMain.push_back(button);


	// Populate Pause Menu
	button = new DungeonMenuButton("ResumeGame", menuResumeGameTexPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteResumeGame, this);
	menuButtonsPause.push_back(button);

	button = new DungeonMenuButton("ReturnToLobby", menuReturnToLobbyPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteReturnToLobby, this);
	menuButtonsPause.push_back(button);

	button = new DungeonMenuButton("ReturnToMenu", menuReturnToMenuTexPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteReturnToMainMenu, this);
	menuButtonsPause.push_back(button);
	menuButtonsThanks.push_back(button);

	button = new DungeonMenuButton("QuitGame", menuQuitTexPath);
	button->BindMenuFunction(&DungeonMenu::ExecuteQuitGame, this);
	menuButtonsPause.push_back(button);
	menuButtonsThanks.push_back(button);

	// Populate Thanks screen
	menuThanksCardTex = TextureManager::GetTexture(menuThanksCardTexPath);

	blackTex = TextureManager::GetTexture(blackTexPath);

	intro01 = TextureManager::GetTexture("crawler/texture/gui/intro/01.tga");
	intro02 = TextureManager::GetTexture("crawler/texture/gui/intro/02.tga");
	intro03 = TextureManager::GetTexture("crawler/texture/gui/intro/03.tga");
	intro04 = TextureManager::GetTexture("crawler/texture/gui/intro/04.tga");
	intro05 = TextureManager::GetTexture("crawler/texture/gui/intro/05.tga");
	introPressSpace = TextureManager::GetTexture("crawler/texture/gui/intro/begin.tga");
	introPressSpacePad = TextureManager::GetTexture("crawler/texture/gui/intro/beginPad.tga");

}

void Crawl::DungeonMenu::OpenMenu(Menu menu)
{
	currentMenu = menu;
	app->s_mode = Application::Mode::Menu;
	Window::GetWindow()->SetMouseCursorHidden(false);
}

void Crawl::DungeonMenu::Update(float delta)
{
	UpdatePositions();

	switch (currentMenu)
	{
	case Menu::Main:
	{
		UpdateInput(&menuButtonsMain);
		DrawMainMenu(delta);
		UpdateMainMenuCamera(delta);
		break;
	}
	case Menu::Pause:
	{
		UpdateInput(&menuButtonsPause);
		DrawPauseMenu(delta);
		break;
	}
	case Menu::Credits:
	{
		DrawCredits(delta);
		break;
	}
	case Menu::Thanks:
	{
		UpdateInput(&menuButtonsThanks);
		DrawThanks(delta);
		break;
	}
	}

}

void Crawl::DungeonMenu::UpdateInput(std::vector<DungeonMenuButton*>* menuButtons)
{
	if (Input::GetLastInputType() != Input::InputType::Mouse)
	{
		int inputDirection = 0;
		if (Input::Alias("Backward").Down()) inputDirection++;
		if (Input::Alias("Forward").Down()) inputDirection--;

		if (inputDirection != 0) selectedMenuOption += inputDirection;
		if (selectedMenuOption < 0) selectedMenuOption = menuButtons->size() - 1;
		if (selectedMenuOption == menuButtons->size()) selectedMenuOption = 0;

		if (Input::Alias("Interact").Down()) menuButtons->at(selectedMenuOption)->Activate();

	}
	else
	{
		for (int i = 0; i < menuButtons->size(); i++)
		{
			if (menuButtons->at(i)->IsMouseOver()) selectedMenuOption = i;
		}

		if (glfwGetMouseButton(Window::Get()->GetGLFWwindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS &&
			menuButtons->at(selectedMenuOption)->IsMouseOver())
		{
			menuButtons->at(selectedMenuOption)->Activate();
		}
	}
}

void Crawl::DungeonMenu::DrawMainMenu(float delta)
{
	UpdatePositions();

	ImGui::SetNextWindowSize({ (float)titleMenuSize.x, (float)titleMenuSize.y });
	ImGui::SetNextWindowPos({ (float)mainMenuXOffset, (float)mainMenuYOffset }, 0, { 0.0f, 1.0f });
	ImGui::Begin("Crawler", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	for (int i = 0; i < menuButtonsMain.size(); i++)
	{
		if (i == selectedMenuOption) menuButtonsMain[i]->Hover();
		menuButtonsMain[i]->Update(delta);
	}


	ImGui::End();
	if (blackScreenFraction > 0.0f)
	{
		DrawBlackScreen(blackScreenFraction, true);
		blackScreenFraction = max(blackScreenFraction - delta, 0.0f);
	}
}

void Crawl::DungeonMenu::DrawPauseMenu(float delta)
{
	if (Input::Keyboard(GLFW_KEY_ESCAPE).Down())
	{
		ExecuteResumeGame();
		return;
	}

	UpdatePositions();
	DrawBlackScreen(0.75f);
	ImGui::SetNextWindowSize({ (float)pauseMenuSize.x, (float)pauseMenuSize.y });
	ImGui::SetNextWindowPos({ (float)mainMenuXOffset, (float)mainMenuYOffset }, 0, { 0.0f, 1.0f });
	ImGui::Begin("Crawler", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	for (int i = 0; i < menuButtonsPause.size(); i++)
	{
		if(i == selectedMenuOption) menuButtonsPause[i]->Hover();
		menuButtonsPause[i]->Update(delta);
	}

	ImGui::Text("");
	if (ImGui::Button(textToggleFullScreen.c_str()))
		ExecuteToggleFullScreen();
	ImGui::Checkbox("Invert Y Axis", &player->invertYAxis);
	if (ImGui::Checkbox("Always Free-Look", &player->alwaysFreeLook))
	{
		player->autoReOrientDuringFreeLook = true;
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("The mouse will act like a traditional first person game");
	if (player->alwaysFreeLook) ImGui::BeginDisabled();
	ImGui::Checkbox("Auto-Orient during Free-Look", &player->autoReOrientDuringFreeLook);
	if (player->alwaysFreeLook) ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("The player will automatically re-orient themselves to the closest cardinal direction when Free-Looking");

	ImGui::End();
}

void Crawl::DungeonMenu::DrawCredits(float delta)
{
}

void Crawl::DungeonMenu::DrawThanks(float delta)
{
	DrawBlackScreen(1.0f);

	// Thanks
	ImGui::SetNextWindowSize({ 850, 650 });
	ImGui::SetNextWindowPos({ screenSize.x/2, 0 }, 0, { 0.5f, 0.0f });
	ImGui::Begin("Thanks", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	DrawImage(menuThanksCardTex, 1.0f);
	ImGui::End();

	// Buttons
	ImGui::SetNextWindowSize({ (float)titleMenuSize.x, (float)titleMenuSize.y });
	ImGui::SetNextWindowPos({ (float)mainMenuXOffset, (float)mainMenuYOffset }, 0, { 0.0f, 1.0f });
	ImGui::Begin("Buttons", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	for (int i = 0; i < menuButtonsThanks.size(); i++)
	{
		if (i == selectedMenuOption) menuButtonsThanks[i]->Hover();
		menuButtonsThanks[i]->Update(delta);
	}
	//if (pauseButtonReturnToMenu->Update(delta))
	//	ExecuteReturnToMainMenu();
	//if (pauseButtonQuit->Update(delta))
	//	ExecuteQuitGame();
	ImGui::End();
}

void Crawl::DungeonMenu::DrawBlackScreen(float alpha, bool onTop)
{
	glm::vec2 size = Window::GetViewPortSize();
	ImGui::SetNextWindowPos({ 0.0f - 100.0f, 0.0f - 100.0f });
	ImGui::SetNextWindowSize({ size.x + 200.0f, size.y + 200.0f });
	if(onTop) ImGui::SetNextWindowFocus();
	ImGui::Begin("Black Fade", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::Image(
		(ImTextureID)blackTex->texID,
		{ size.x + 200.0f, size.y + 200.0f },
		{ 0,0 },
		{ 1,1 },
		{ 1,1,1, alpha });
	ImGui::End();
}

void Crawl::DungeonMenu::DrawImage(Texture* tex, float alpha)
{
	ImGui::Image(
		(ImTextureID)tex->texID,
		{ (float)tex->width, (float)tex->height },
		{ 0,1 },
		{ 1,0 },
		{ 1,1,1, alpha });
}

void Crawl::DungeonMenu::UpdatePositions()
{
	window = Window::Get();
	screenSize = window->GetViewPortSize();
	mainMenuXOffset = screenSize.x / 10;
	mainMenuYOffset = (screenSize.y / 10) * 9;
}

void Crawl::DungeonMenu::UpdateMainMenuCamera(float delta)
{
	// hover float
	noiseAccumulator += delta;
	vec3 camPos;
	camPos.x = menuCameraPositionMax.x * glm::perlin(glm::vec2(0.1f, (noiseAccumulator + menuCameraPositionAccumulatorOffset.x) * menuCameraPositionFrequency.x));
	camPos.y = menuCameraPositionMax.y * ((glm::perlin(glm::vec2(0.1f, (noiseAccumulator + menuCameraPositionAccumulatorOffset.y) * menuCameraPositionFrequency.y)) - 0.5f) * 2.0f);
	camPos.z = menuCameraPositionMax.z * ((glm::perlin(glm::vec2(0.1f, (noiseAccumulator + menuCameraPositionAccumulatorOffset.z) * menuCameraPositionFrequency.z)) - 0.5f) * 2.0f);
	cameraObject->children[0]->SetLocalPosition(camPos);

	vec3 camRot;
	camRot.x = menuCameraRotationMax.x * glm::perlin(glm::vec2(0.1f, (noiseAccumulator + menuCameraRotationAccumulatorOffset.x) * menuCameraRotationFrequency.x));
	camRot.y = menuCameraRotationMax.y * ((glm::perlin(glm::vec2(0.1f, (noiseAccumulator + menuCameraRotationAccumulatorOffset.y) * menuCameraRotationFrequency.y)) - 0.5f) * 2.0f);
	camRot.z = menuCameraRotationMax.z * ((glm::perlin(glm::vec2(0.1f, (noiseAccumulator + menuCameraRotationAccumulatorOffset.z) * menuCameraRotationFrequency.z)) - 0.5f) * 2.0f);
	cameraObject->children[0]->SetLocalRotation(camRot);

	// sequence
	if (!newGameSequenceStarted)
		return;

	if (Input::Alias("Interact").Down() && newGameSequenceTime > 1.0f) StartNewGame();

	newGameSequenceTime += delta;
	switch (introStage)
	{
	case IntroStage::Idle:
	{
		if (newGameSequenceTime > 1.0f)
		{
			introStage = IntroStage::Turn;
			newGameSequenceTime = 0.0f;
		}
		break;
	}
	case IntroStage::Turn:
	{
		float t = newGameSequenceTime / 7.0f;
		float tClamped = glm::clamp(t, 0.0f, 1.0f);
		cameraObject->SetLocalRotationZ(MathUtils::Lerp(-90.0f, -270.0f, glm::sineEaseInOut(tClamped)));

		if (t >= 1.05f)
		{
			introStage = IntroStage::Fly;
			newGameSequenceTime = 0.0f;
		}
		break;
	}
	case IntroStage::Fly:
	{
		float spinT = glm::clamp(newGameSequenceTime / 1.0f, 0.0f, 1.0f);
		float moveT = glm::clamp(newGameSequenceTime / 1.0f, 0.0f, 1.0f);
		vec3 newPos = cameraObject->localPosition;
		newPos.x = MathUtils::Lerp(0.0f, 22.0f, moveT);
		vec3 newRot = { 0,0,90.0f };
		newRot.y = MathUtils::Lerp(0.0f, 45, spinT);
		cameraObject->SetLocalPosition(newPos);
		cameraObject->SetLocalRotation(newRot);

		if (moveT > 0.5f)
		{
			DrawBlackScreen((moveT - 0.5f) * 2.0f);
		}
		if (moveT >= 1.0f)
		{
			introStage = IntroStage::Text;
			newGameSequenceTime = 0.0f;
		}

		break;
	}
	case IntroStage::Text:
	{
		DrawBlackScreen(1.0f);
		if (newGameSequenceTime > 2.0f)
		{
			ImGui::SetNextWindowPos({ 50, 50 });
			ImGui::SetNextWindowSize({ 1920,1080 });
			ImGui::Begin("Intro Text", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
			float workingNum = newGameSequenceTime - 2;
			DrawImage(intro01, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 4;
			DrawImage(intro02, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 6;
			DrawImage(intro03, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 8;
			DrawImage(intro04, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 10;
			DrawImage(intro05, glm::clamp(workingNum, 0.0f, 1.0f));
			ImGui::End();
			
			glm::vec2 screenSize = Window::GetViewPortSize();
			ImGui::SetNextWindowPos({ screenSize.x/2.0f, screenSize.y - 50 }, 0, {0.5,1.0f});
			ImGui::SetNextWindowSize({ (float)introPressSpace->width+15, (float)introPressSpace->height });
			ImGui::Begin("Press Space Text", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
			workingNum = newGameSequenceTime - 12;
			if(Input::GetLastInputType() == Input::InputType::Gamepad)
				DrawImage(introPressSpacePad, glm::clamp(workingNum, 0.0f, 1.0f));
			else
				DrawImage(introPressSpace, glm::clamp(workingNum, 0.0f, 1.0f));
			ImGui::End();
		}
		break;
	}
	}
}

void Crawl::DungeonMenu::ExecuteNewGameButton()
{	
	newGameSequenceStarted = true;
	for (auto& menuButton : menuButtonsMain) menuButton->SetEnabled(false);

	Window::GetWindow()->SetMouseCursorHidden(true);
	AudioManager::StopMusic();
}

void Crawl::DungeonMenu::StartNewGame()
{
	app->s_mode = Application::Mode::Game;
	player->currentDungeon->Load("crawler/dungeon/start.dungeon");
	player->ResetPlayer();
	player->Respawn();
	Scene::SetCameraByName("Player Camera");
	cameraObject->markedForDeletion = true;
}

void Crawl::DungeonMenu::ExecuteQuitGame()
{
	glfwSetWindowShouldClose(window->GetGLFWwindow(), GLFW_TRUE);
}

void Crawl::DungeonMenu::ExecuteResumeGame()
{
	currentMenu = Menu::None;
	app->s_mode = Application::Mode::Game;
	Window::GetWindow()->SetMouseCursorHidden(true);
	player->SetStateIdle();;
}

void Crawl::DungeonMenu::ExecuteToggleFullScreen()
{
	Window::GetWindow()->ToggleFullscreen();
}

void Crawl::DungeonMenu::ExecuteReturnToLobby()
{
	currentMenu = Menu::None;
	app->s_mode = Application::Mode::Game;
	Window::GetWindow()->SetMouseCursorHidden(true);
	player->SetStateIdle();
	player->ClearCheckpoint();
	player->ReturnToLobby();
}

void Crawl::DungeonMenu::ExecuteReturnToMainMenu()
{
	DungeonGameManager::Get()->ResetGameState();
	app->dungeon->Load("crawler/dungeon/lobby.dungeon");
	player->ResetPlayer();
	player->SetLevel2(false);
	player->Teleport({ 11, -2 });

	introStage = IntroStage::Idle;
	newGameSequenceTime = 0.0f;
	noiseAccumulator = 0.0f;
	blackScreenFraction = 1.0f;
	lobbyReturnEnabled = false;
	newGameSequenceStarted = false;
	newGameSequenceTime = 0.0f;
	app->s_mode = Application::Mode::Menu;
	currentMenu = Menu::Main;

	for (auto& menuButton : menuButtonsMain) menuButton->SetEnabled();

	Object* camera = Scene::CreateObject();
	camera->LoadFromJSON(ReadJSONFromDisk("crawler/object/menu_camera.object"));
	SetMenuCameraObject(camera);
	Scene::SetCameraByName("Camera");
	DrawBlackScreen(1.0f);

	AudioManager::ChangeMusic(menuMusic);
}
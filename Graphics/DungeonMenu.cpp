#include "DungeonMenu.h"
#include "DungeonMenuButton.h"
#include "DungeonPlayer.h"
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

Crawl::DungeonMenu::DungeonMenu()
{
	buttonNewGame = new DungeonMenuButton("New Game", menuNewGameTexPath);
	buttonSettings = new DungeonMenuButton("Settings", menuSettingsTexPath);
	buttonQuit = new DungeonMenuButton("QuitGame", menuQuitTexPath);

	pauseButtonResumeGame = new DungeonMenuButton("ResumeGame", menuResumeGameTexPath);
	pauseButtonReturnLobby = new DungeonMenuButton("ReturnToLobby", menuReturnToLobbyPath);
	pauseButtonReturnToMenu = new DungeonMenuButton("ReturnToMenu", menuReturnToMenuTexPath);
	pauseButtonQuit = new DungeonMenuButton("QuitGame", menuQuitTexPath);


	blackTex = TextureManager::GetTexture(blackTexPath);

	intro01 = TextureManager::GetTexture("crawler/texture/gui/intro/01.tga");
	intro02 = TextureManager::GetTexture("crawler/texture/gui/intro/02.tga");
	intro03 = TextureManager::GetTexture("crawler/texture/gui/intro/03.tga");
	intro04 = TextureManager::GetTexture("crawler/texture/gui/intro/04.tga");
	intro05 = TextureManager::GetTexture("crawler/texture/gui/intro/05.tga");
	introPressSpace = TextureManager::GetTexture("crawler/texture/gui/intro/begin.tga");
}

void Crawl::DungeonMenu::Update(float delta)
{
	DrawMainMenu(delta);
	UpdateMainMenuCamera(delta);
}

void Crawl::DungeonMenu::DrawMainMenu(float delta)
{
	UpdatePositions();

	ImGui::SetNextWindowSize({ (float)titleMenuSize.x, (float)titleMenuSize.y });
	ImGui::SetNextWindowPos({ (float)mainMenuXOffset, (float)mainMenuYOffset }, 0, { 0.0f, 1.0f });
	ImGui::Begin("Crawler", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	if (buttonNewGame->Update(delta))
	{
		ExecuteNewGameButton();
		// do stuff
	}
	//if (ImGui::Button(textToggleFullScreen.c_str()))
	//{
	//	ExecuteToggleFullScreen();
	//	// do stuff
	//}
	if (buttonQuit->Update(delta))
	{
		ExecuteQuitGame();
		// do stuff
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
	
	if (pauseButtonResumeGame->Update(delta))
		ExecuteResumeGame();
	if (lobbyReturnEnabled)
	{
		if (pauseButtonReturnLobby->Update(delta))	ExecuteReturnToLobby();
	}
	if (pauseButtonReturnToMenu->Update(delta))
		ExecuteReturnToMainMenu();
	if (pauseButtonQuit->Update(delta))
		ExecuteQuitGame();

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

void Crawl::DungeonMenu::DrawBlackScreen(float alpha, bool onTop)
{
	glm::vec2 size = Window::GetViewPortSize();
	ImGui::SetNextWindowPos({ 0.0f - 100.0f, 0.0f - 100.0f });
	ImGui::SetNextWindowSize({ size.x + 200.0f, size.y + 200.0f });
	if(onTop) ImGui::SetNextWindowFocus();
	ImGui::Begin("Black Fade", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
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
	glm::ivec2 size = window->GetViewPortSize();
	mainMenuXOffset = size.x / 10;
	mainMenuYOffset = (size.y / 10) * 9;
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
		cameraObject->SetLocalRotationZ(MathUtils::Lerp(-90.0f, 90.0f, glm::sineEaseInOut(tClamped)));

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
		if (newGameSequenceTime > 3.0f)
		{
			ImGui::SetNextWindowPos({ 50, 50 });
			ImGui::SetNextWindowSize({ 1920,1080 });
			ImGui::Begin("Intro Text", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
			float workingNum = newGameSequenceTime - 3;
			DrawImage(intro01, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 5;
			DrawImage(intro02, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 7;
			DrawImage(intro03, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 9;
			DrawImage(intro04, glm::clamp(workingNum, 0.0f, 1.0f));
			workingNum = newGameSequenceTime - 11;
			DrawImage(intro05, glm::clamp(workingNum, 0.0f, 1.0f));
			ImGui::End();
			
			glm::vec2 screenSize = Window::GetViewPortSize();
			ImGui::SetNextWindowPos({ screenSize.x/2.0f, screenSize.y - 100 }, 0, {0.5,1.0f});
			ImGui::SetNextWindowSize({ (float)introPressSpace->width, (float)introPressSpace->height });
			ImGui::Begin("Press Space Text", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
			workingNum = newGameSequenceTime - 13.5;
			DrawImage(introPressSpace, glm::clamp(workingNum, 0.0f, 1.0f));
			ImGui::End();
		}
		if(Input::Keyboard(GLFW_KEY_SPACE).Down()) StartNewGame();
		break;
	}
	}
}

void Crawl::DungeonMenu::ExecuteNewGameButton()
{	
	newGameSequenceStarted = true;
	buttonNewGame->SetActive(false);
	buttonSettings->SetActive(false);
	buttonQuit->SetActive(false);
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
	Window::GetWindow()->SetMouseCursorHidden(true);
	player->SetStateIdle();;
}

void Crawl::DungeonMenu::ExecuteToggleFullScreen()
{
	Window::GetWindow()->ToggleFullscreen();
}

void Crawl::DungeonMenu::ExecuteReturnToLobby()
{
	Window::GetWindow()->SetMouseCursorHidden(true);
	player->SetStateIdle();
	player->ClearCheckpoint();
	player->ReturnToLobby();
}

void Crawl::DungeonMenu::ExecuteReturnToMainMenu()
{
	app->dungeon->Load("crawler/dungeon/lobby.dungeon");
	player->SetStateIdle();
	player->Teleport({ 11, -2 });
	player->ClearCheckpoint();

	introStage = IntroStage::Idle;
	newGameSequenceTime = 0.0f;
	noiseAccumulator = 0.0f;
	blackScreenFraction = 1.0f;
	lobbyReturnEnabled = false;
	newGameSequenceStarted = false;
	newGameSequenceTime = 0.0f;
	app->s_mode = Application::Mode::MainMenu;

	buttonNewGame->SetActive();
	buttonSettings->SetActive();
	buttonQuit->SetActive();

	Object* camera = Scene::CreateObject();
	camera->LoadFromJSON(ReadJSONFromDisk("crawler/object/menu_camera.object"));
	SetMenuCameraObject(camera);
	Scene::SetCameraByName("Camera");
	DrawBlackScreen(1.0f);

	AudioManager::ChangeMusic(menuMusic);
}
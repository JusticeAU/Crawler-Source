#include "DungeonGameManager.h"
#include "DungeonEditor.h"
#include "DungeonGameManagerEvent.h"
#include "DungeonPlayer.h"
#include "DungeonDoor.h"
#include "DungeonLight.h"
#include "DungeonTransporter.h"
#include "Scene.h"
#include "MathUtils.h"
#include "gtx/easing.hpp"

#include "LogUtils.h"
#include "AudioManager.h"
#include "DungeonMenu.h"
#include "Application.h"

#include "TextureManager.h"
#include "SceneRenderer.h"

#include "Input.h"

Crawl::DungeonGameManager* Crawl::DungeonGameManager::instance = nullptr;

Crawl::DungeonGameManager::DungeonGameManager()
{
	instance = this;
	// Generate the padlock Z positions based on the hinges.
	for (int i = 0; i < 4; i++) frontDoorPadlockZPositions[i] = frontDoorHingeZPositions[i] + frontDoorPadlockZOffset;
}

void Crawl::DungeonGameManager::Init()
{
	if (instance == nullptr) instance = new DungeonGameManager();
}

bool Crawl::DungeonGameManager::DrawGUI()
{
	ImGui::Begin("Game Manager");
	bool changed = DrawGUIInternal(); // Editor can call this only to inject it in to its interface.
	ImGui::End();
	return changed;
}

bool Crawl::DungeonGameManager::DrawGUIInternal()
{
	bool changed = false;
	if (ImGui::Checkbox("Manage Lobby", &manageLobby)) changed = true;
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Enables whether the GameManager should configure the lobby. If this is disabled then all doors in the lobby will be open.");
	ImGui::PushID("Doors");
	ImGui::Text("Door States");
	for (int i = 0; i < 8; i++)
	{
		ImGui::PushID(i);
		int current = (int)doorStates[i];
		if (ImGui::SliderInt(doorNames[i].c_str(), &current, 0, 2, doorStateNames[current].c_str(), ImGuiSliderFlags_AlwaysClamp))
		{
			doorStates[i] = (DoorState)current;
			changed = true;
		}
		ImGui::PopID();
	}
	ImGui::PopID();

	ImGui::PushID("Lights");
	ImGui::Text("Lights Enabled");
	for (int i = 0; i < 8; i++)
	{
		ImGui::PushID(i);
		if(ImGui::Checkbox(doorNames[i].c_str(), &enabledLights[i]) ) changed = true;
		if (i % 2 == 0) ImGui::SameLine();
		ImGui::PopID();
	}
	ImGui::PopID();

	ImGui::PushID("Locks");
	ImGui::Text("Front Door Locks");
	for (int i = 0; i < 4; i++)
	{
		ImGui::PushID(i);
		if(ImGui::Checkbox(std::to_string(i+1).c_str(), &frontDoorUnlocked[i])) changed = true;
		if (i < 3) ImGui::SameLine();
		ImGui::PopID();
	}
	ImGui::PopID();
	ImGui::Text("Debug stuff");
	if(ImGui::Button("Trigger animations")) UpdateDoorStateEvent();
	ImGui::PushID("Door Rotations");
	if (frontDoorLeft) // door is configured
	{
		for (int i = 0; i < 4; i++)
		{
			ImGui::PushID(i);
			float latchAxis = frontDoorLocksLatches[i]->localRotation.z;
			if (ImGui::SliderFloat("Latch Open", &latchAxis, -180, 0)) frontDoorLocksLatches[i]->SetLocalRotation({ 0,0, latchAxis });
			ImGui::PopID();
		}

		if (ImGui::SliderFloat("Door Open", &doorSwingAmount, 180, 0))
		{
			frontDoorLeft->SetLocalRotation({ 0,0,90 - doorSwingAmount });
			frontDoorRight->SetLocalRotation({ 0,0,90 + doorSwingAmount });

		}
	}
	ImGui::PopID();
	return changed;
}

void Crawl::DungeonGameManager::Update(float delta)
{
	UpdateFTUE(delta);
	
	if (player->GetDungeonLoaded()->isLobby)
	{
		UpdateLobbyVisuals(delta);
		UpdateDoorStateEvent();
	}

	if (startVoid)
		UpdateVoidVisuals(delta);
}

void Crawl::DungeonGameManager::PauseGame()
{
	menu->OpenMenu(DungeonMenu::Menu::Pause);
}

void Crawl::DungeonGameManager::UnpauseGame()
{
	player->SetStateIdle();
}

void Crawl::DungeonGameManager::ReturnToEditor()
{
	Application::s_mode = Application::Mode::Design;
	editor->Activate();
}

void Crawl::DungeonGameManager::ResetGameState()
{
	// this is gross!
	for (int i = 0; i < 8; i++) doorStates[i] = DoorState::Closed;
	doorStates[0] = DoorState::Open;
	
	for (int i = 0; i < 8; i++) enabledLights[i] = false;
	enabledLights[0] = true;

	lobbyHasTriggeredLightning = false;
	frontDoorUpdateTriggered = false;
	for (int i = 0; i < 4; i++)
	{
		frontDoorUnlocked[i] = false;
		frontDoorUnlockAnimationStarted[i] = false;
		frontDoorUnlockHingeT[i] = 0;
		frontDoorUnlockShackleT[i] = 0;
		frontDoorUnlockPadlockT[i] = 0;
	}

	frontDoorOpenAnimationStarted = false;
	frontDoorOpenT = 0.0f;
}

void Crawl::DungeonGameManager::RunGMEvent(const DungeonGameManagerEvent& gme)
{
	switch (gme.type)
	{
	case DungeonGameManagerEvent::Type::Door:
	{
		switch ((DoorState)gme.status)
		{
		case DoorState::Open:
		{
			OpenDoor(gme.id);
			EnableLight(gme.id);
			break;
		}
		case DoorState::Closed:
		{
			Get()->CloseDoor(gme.id);
			Get()->DisableLight(gme.id);
			break;
		}
		case DoorState::Barricaded:
		{
			BarricadeDoor(gme.id);
			DisableLight(gme.id);
			break;
		}
		}
		break;
	}
	case DungeonGameManagerEvent::Type::Lock:
	{
		switch (gme.status)
		{
		case 0: // Off
		{
			RemoveFrontDoorLock(gme.id);
			break;
		}
		case 1: // On 
		{
			// this is not a thing
			break;
		}
		}
		break;
	}

	}
}

void Crawl::DungeonGameManager::ClearLocksObject()
{
	// Clear up scene objects
	if (frontDoorLocksSceneObject)
		frontDoorLocksSceneObject->markedForDeletion = true;

	frontDoorLocksSceneObject = nullptr;
	frontDoorLeft = nullptr;
	frontDoorRight = nullptr;
	for (int i = 0; i < 4; i++) // All purpose loop for doing things 4 times!
	{
		frontDoorLocksLatches[i] = nullptr;
		if (frontDoorUnlockAnimationStarted[i]) frontDoorUnlockHingeT[i] == 1.0f; // ensure these have finished.
		
		if (frontDoorPadLockObjects[i])
		{
			frontDoorPadLockObjects[i]->markedForDeletion = true;
			frontDoorPadLockObjects[i] = nullptr;
		}
	
	}

	// Clear animation flags for next time we approach the door
	frontDoorUpdateTriggered = false;
}

void Crawl::DungeonGameManager::ConfigureLobby()
{
	Dungeon* lobby1 = player->GetDungeonLoaded();
	Dungeon* lobby2 = player->GetDungeonLobbyLevel2();
	
	// Get Light References
	lobbyLightingLight = lobby1->GetLightWithID(lobbyLightningLightID);
	for (int i = 0; i < 8; i++)
	{
		doorLights[i] = lobby1->GetLightWithID(i + 1);
	}

	// Get Door references
	doors[0] = lobby1->GetDoorWithID(2);
	doors[1] = lobby1->GetDoorWithID(3);
	doors[2] = lobby2->GetDoorWithID(1);
	doors[3] = lobby2->GetDoorWithID(2);
	doors[4] = lobby2->GetDoorWithID(3);
	doors[5] = lobby2->GetDoorWithID(4);
	doors[6] = lobby2->GetDoorWithID(5);
	doors[7] = lobby2->GetDoorWithID(6);

	if (manageLobby)
	{
		// Configure Doors
		for (int i = 0; i < 8; i++)
		{
			if (doors[i])
			{
				switch (doorStates[i])
				{
				case DoorState::Open:
					doors[i]->Open(true);
					break;
				case DoorState::Closed:
					doors[i]->Close(true);
					break;
				case DoorState::Barricaded:
					doors[i]->MakeBarricaded();
					break;
				}
			}

			if (enabledLights[i])
			{
				lobby1->EnableLights(i + 1);
				lobby1->FlickerLights(i + 1);
				lobby1->SetLightFlickerLoop(i + 1, 0, 4);
				//doorLights[i]->Enable();
				//doorLights[i]->Flicker();
				//doorLights[i]->flickerRepeat = true;
				//doorLights[i]->flickerRepeatMin = 0;
				//doorLights[i]->flickerRepeatMax = 4;
			}
			else if (!enabledLights[i])
			{
				lobby1->DisableLights(i + 1);
			}
		}
		// Configure level 1 doors
		// Configure level 2 doors

		ConfigureLobbyDoor();
	}
}

void Crawl::DungeonGameManager::ConfigureLobbyDoor()
{
	// configure the main door status (locks etc??)
	frontDoorLocksSceneObject = Scene::CreateObject();
	frontDoorLocksSceneObject->LoadFromJSON(ReadJSONFromDisk(frontDoorLocksObjectFilePath));
	// doors
	frontDoorLocksSceneObject->children[0]->children[0]->children[0]->LoadFromJSON(ReadJSONFromDisk(frontDoorLocksLeftDoor));
	frontDoorLeft = frontDoorLocksSceneObject->children[0];

	frontDoorLocksSceneObject->children[1]->children[0]->children[0]->LoadFromJSON(ReadJSONFromDisk(frontDoorLocksRightDoor));
	frontDoorRight = frontDoorLocksSceneObject->children[1];

	// locks
	for (int i = 0; i < 4; i++)
	{
		frontDoorLocksSceneObject->children[0]->children[0]->children[i + 1]->children[0]->LoadFromJSON(ReadJSONFromDisk(frontDoorLocksLatch));
		frontDoorLocksLatches[i] = frontDoorLocksSceneObject->children[0]->children[0]->children[i + 1]->children[0];
		frontDoorLocksSceneObject->children[0]->children[0]->children[i + 1]->children[1]->LoadFromJSON(ReadJSONFromDisk(frontDoorLocksBracket));
		frontDoorLocksSceneObject->children[1]->children[0]->children[i + 1]->children[0]->LoadFromJSON(ReadJSONFromDisk(frontDoorLocksPadEye));
		
		if(frontDoorUnlockAnimationStarted[i]) // Catch all for locks that have unlocked
			frontDoorLocksLatches[i]->SetLocalRotationZ(frontDoorHingePosEnd);
		else
		{
			// load the lock asset
			frontDoorPadLockObjects[i] = Scene::CreateObject();
			frontDoorPadLockObjects[i]->LoadFromJSON(ReadJSONFromDisk(frontDoorPadLockObjectPath));
			vec3 pos = frontDoorPadLockObjects[i]->localPosition;
			pos.z = frontDoorPadlockZPositions[i];
			frontDoorPadLockObjects[i]->SetLocalPosition(pos);

			frontDoorPadLockShackleObjects[i] = frontDoorPadLockObjects[i]->children[0];
			Object* shackleModel = Scene::CreateObject(frontDoorPadLockShackleObjects[i]->children[0]);
			shackleModel->LoadFromJSON(ReadJSONFromDisk(frontDoorPadLockShacklePath));

			Object* padlockModel = Scene::CreateObject(frontDoorPadLockObjects[i]->children[1]->children[0]);
			padlockModel->LoadFromJSON(ReadJSONFromDisk(frontDoorPadLockPadlockPath));
		}
	}
}
// This is triggered when the player is facing west (toward the main door) and within a particular quadrant
void Crawl::DungeonGameManager::UpdateDoorStateEvent()
{
	// we check this once per step and make sure they are in the right spot.
	if (frontDoorUpdateTriggered) return;

	if (player->GetOrientation() != WEST_INDEX) return;
	ivec2 playerPos = player->GetPosition();
	if (!(
		playerPos.x >= frontDoorUnlockPositionMin.x && playerPos.x <= frontDoorUnlockPositionMax.x &&
		playerPos.y >= frontDoorUnlockPositionMin.y && playerPos.y <= frontDoorUnlockPositionMax.y)) return;

	frontDoorUpdateTriggered = true;
	for (int i = 0; i < 4; i++)
	{
		if (frontDoorUnlocked[i] && !frontDoorUnlockAnimationStarted[i])
		{
			// Update look animation state
			frontDoorUnlockAnimationStarted[i] = true;
			
			// Play SFX
		}
	}
}

void Crawl::DungeonGameManager::MakeLobbyExitTraversable()
{
	DungeonTile* tile = player->currentDungeon->GetTile(positionToMakeTraversable);
	if (tile)
	{
		if((tile->maskTraverse & WEST_MASK) != WEST_MASK)
			tile->maskTraverse += WEST_MASK;
	}
}

void Crawl::DungeonGameManager::MakeLobbyVoidUnTraversable()
{
	DungeonTile* tile = player->currentDungeon->GetTile(positionToMakeUntraversable);
	if (tile)
	{
		tile->maskTraverse = 0;
	}
}

void Crawl::DungeonGameManager::DoEvent(int eventID)
{
	switch (eventID)
	{
	case -1:
	{
		LogUtils::Log("Attempted to trigger unassigned event (-1)");
		return;
	}
	case 0: // No Longer In Use.
	{

		return;
	}
	case 1: // activate lighting in lobby
	{
		ActivateLobbyLight();
		return;
	}
	case 2: // clear FTUE Event
	{
		//player->ClearFTUEPrompt();
		return;
	}
	case 3: // trigger Move FTUE
	{
		//player->SetFTUEPrompt(promptMove);
		return;
	}

	case 4: // trigger look prompt
	{
		//if (!player->ftueHasLooked) player->SetFTUEPrompt(promptLook);
		return;
	}
	case 5: // trigger Interact Prompt
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Interact);
		return;
	}
	case 6: // Trigger Wait prompt
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Wait);
		return;
	}
	case 7: // Trigger Reset prompt
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Reset);
		return;
	}
	case 50: // void sequence
	{
		//menu->OpenMenu(DungeonMenu::Menu::Thanks);
		MakeLobbyVoidUnTraversable();
		startVoid = true;
		voidTrigger = true;
		player->SetLanternEmissionScale(0.0f);
		player->SetLightIntensity(0.0f);
		SceneRenderer::ambient = 0.0f;
		AudioManager::PlaySound("crawler/sound/load/void/lights_out.wav");
		player->useRespawnSound = false;
		return;
	}
	case 51: // reached void checkpoint
	{
		voidTrigger = true;
		voidLights += 1;
		player->GetDungeonLoaded()->DisableLights(1);
		player->GetDungeonLoaded()->GetTile(player->GetPosition())->maskTraverse = 0;
		AudioManager::PlaySound("crawler/sound/load/void/fizzle1.wav");

		// turn light off in level
		return;
	}
	default:
	{
		LogUtils::Log("Attempted to trigger event that doesnt exist (" + std::to_string(eventID) + ")");
		return;
	}
	}
}

void Crawl::DungeonGameManager::UpdateLobbyVisuals(float delta)
{
	UpdateLobbyVisualsLightning(delta);
	if(manageLobby) UpdateLobbyVisualsLocks(delta);
}

void Crawl::DungeonGameManager::UpdateLobbyVisualsLightning(float delta)
{
	// lightning strike
// This gotta be moved to some game / global event manager
	if (lobbyHasTriggeredLightning && lobbyLightingLight != nullptr && lobbyLightningTimeCurrent < (lobbyLightningStrikeTime + 0.15f))
	{
		float t = glm::bounceEaseIn(glm::clamp(lobbyLightningTimeCurrent / lobbyLightningStrikeTime, 0.0f, 1.0f));
		t = glm::clamp(t, 0.0f, 1.0f);
		lobbyLightingLight->intensityCurrent = MathUtils::Lerp(0.0f, 1000.0f, t);
		lobbyLightingLight->UpdateLight();

		if (t > 0.3f && !playedSfx)
		{
			AudioManager::PlaySound(lightningSfx);
			playedSfx = true;
		}
		lobbyLightningTimeCurrent += delta;

	}
	else
	{
		lobbyLightningTimeCurrent = (-rand() % 15) - 5.0f;
		playedSfx = false;
		if (lobbyLightingLight != nullptr)
		{
			lobbyLightingLight->intensityCurrent = 0.0f;
			lobbyLightingLight->UpdateLight();
		}
	}
}

void Crawl::DungeonGameManager::UpdateLobbyVisualsLocks(float delta)
{
	for (int i = 0; i < 4; i++)
	{
		if (!frontDoorUnlockAnimationStarted[i]) continue;
		if (frontDoorUnlockHingeT[i] == 1.0f) continue;

		// Do Padlock open
		if(frontDoorUnlockShackleT[i] == 0.0f)
		{
			AudioManager::PlaySound("crawler/sound/load/lobby/maindoor_latchopen.wav", frontDoorLocksSceneObject->GetWorldSpacePosition());
		}
		if (frontDoorUnlockShackleT[i] != 1.0f)
		{
			float lockDelta = (1 / frontDoorPadLockOpenSpeed) * delta;
			float t = frontDoorUnlockShackleT[i] = min(frontDoorUnlockShackleT[i] + lockDelta, 1.0f);
			if (t > 0.85f) frontDoorUnlockShackleT[i] = t = 1.0f;
			float easedT = glm::bounceEaseOut(t);
			frontDoorPadLockShackleObjects[i]->localRotation.y = MathUtils::LerpDegrees(0.0f, frontDoorPadLockOpenAngle, easedT);
			frontDoorPadLockShackleObjects[i]->SetDirtyTransform();
			if (t < 0.5) break;
		}
		

		// Do Padlock fall
		if (frontDoorUnlockPadlockT[i] == 0.0f)
		{
			AudioManager::PlaySound("crawler/sound/load/lobby/maindoor_lockfallthud.wav", frontDoorLocksSceneObject->GetWorldSpacePosition());
		}
		if (frontDoorUnlockPadlockT[i] != 1.0f)
		{
			float fallDelta = (1 / frontDoorPadLockFallSpeed) * delta;
			float t = frontDoorUnlockPadlockT[i] = min(frontDoorUnlockPadlockT[i] + fallDelta, 1.0f);
			float easedT = glm::sineEaseIn(t);
			frontDoorPadLockObjects[i]->localPosition.z = MathUtils::Lerp(frontDoorPadlockZPositions[i], -0.1, easedT);
			frontDoorPadLockObjects[i]->SetDirtyTransform();
			if(t < 0.2)	break;
		}


		// Do Hinge
		if (frontDoorUnlockHingeT[i] != 1.0f)
		{
			float hingeDelta = (1 / frontDoorHingeOpenSpeed) * delta;
			float t = frontDoorUnlockHingeT[i] = min(frontDoorUnlockHingeT[i] + hingeDelta, 1.0f);
			if (t > 0.85f) frontDoorUnlockHingeT[i] = t = 1.0f;
			float tEased = glm::bounceEaseOut(t);
			frontDoorLocksLatches[i]->SetLocalRotationZ(MathUtils::LerpDegrees(frontDoorHingePosStart, frontDoorHingePosEnd, tEased));

			// Check door should unlock.
			if (i == 3 && t == 1.0f)
			{
				frontDoorOpenAnimationStarted = true;
				AudioManager::PlaySound("crawler/sound/load/lobby/maindoor_open2.wav", frontDoorLocksSceneObject->GetWorldSpacePosition());
			}
			break;
		}
	}

	// Do front Door
	if (frontDoorOpenAnimationStarted && frontDoorOpenT != 1.0f)
	{
		float doorDelta = (1 / frontDoorOpenSpeed) * delta;
		float t = frontDoorOpenT = min(frontDoorOpenT + doorDelta, 1.0f);
		float tEased = glm::bounceEaseOut(t);
		frontDoorLeft->SetLocalRotationZ(MathUtils::LerpDegrees(frontDoorOpenRotationStart, frontDoorOpenRotationStart-frontDoorOpenRotationEnd, tEased));
		frontDoorRight->SetLocalRotationZ(MathUtils::LerpDegrees(frontDoorOpenRotationStart, frontDoorOpenRotationStart+frontDoorOpenRotationEnd, tEased));
		
		if (t > 0.5f) MakeLobbyExitTraversable();
	}
}

void Crawl::DungeonGameManager::UpdateVoidVisuals(float delta)
{
	voidTimer += delta;

	if (voidTimer > voidSoundTime)
	{
		AudioManager::PlaySound("crawler/sound/load/start.wav");
		voidTimer -= voidSoundTime;
	}
	
	if (!voidTrigger) return;
	voidTriggerTimer += delta;

	switch (voidState)
	{
	case VoidState::HoldInDoor:
	{
		if (voidTriggerTimer > voidEnterTime)
		{
			AudioManager::ChangeMusic("crawler/sound/stream/void.ogg");
			Scene::SetClearColour({ 0,0,0 });
			DungeonTransporter* transporter = new DungeonTransporter();
			transporter->toDungeon = "crawler/dungeon/levelVoid/void1";
			transporter->toTransporter = "1";
			player->LoadSelectedTransporter(transporter);
			delete transporter;

			voidState = VoidState::LightsOut;
			voidTriggerTimer = 0.0f;
			voidTrigger = false;
		}
		break;
	}
	case VoidState::LightsOut:
	{
		if (voidTriggerTimer > voidLightsOutTime)
		{
			DungeonTransporter* transporter;
			if (voidLights != 4)
			{
				transporter = new DungeonTransporter();
				transporter->toDungeon = "crawler/dungeon/levelVoid/void1";
				transporter->toTransporter = to_string(voidLights);
			}
			else
			{
				transporter = new DungeonTransporter();
				transporter->toDungeon = "crawler/dungeon/levelVoid/void2";
				transporter->toTransporter = "Spawn";
				voidState = VoidState::Chasers;
			}
			player->LoadSelectedTransporter(transporter);
			delete transporter;
			
			voidTriggerTimer = 0.0f;
			voidTrigger = false;
		}
		break;
	}
	case VoidState::Chasers:
	{
		break;
	}

	}

	/*if (Input::Keyboard(GLFW_KEY_T).Down())
	{
		player->currentDungeon->DisableLights(1);

	}*/
}

void Crawl::DungeonGameManager::QueueFTUEPrompt(DungeonGameFTUE::FTUEType type)
{
	bool useGamepadPrompt = (Input::GetLastInputType() == Input::InputType::Gamepad);
	Texture* tex = nullptr;
	switch (type)
	{
	case DungeonGameFTUE::FTUEType::Turn:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftueTurnPad);
		else tex = TextureManager::GetTexture(ftueTurn);
		break;
	}
	case DungeonGameFTUE::FTUEType::Move:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftueMovePad);
		else tex = TextureManager::GetTexture(ftueMove);
		break;
	}
	case DungeonGameFTUE::FTUEType::Interact:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftueInteractPad);
		else tex = TextureManager::GetTexture(ftueInteract);
		break;
	}
	case DungeonGameFTUE::FTUEType::Look:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftueLookPad);
		else tex = TextureManager::GetTexture(ftueLook);
		break;
	}
	case DungeonGameFTUE::FTUEType::Wait:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftueWaitPad);
		else tex = TextureManager::GetTexture(ftueWait);
		break;
	}
	case DungeonGameFTUE::FTUEType::Reset:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftueResetPad);
		else tex = TextureManager::GetTexture(ftueReset);
		break;
	}
	case DungeonGameFTUE::FTUEType::Push:
	{
		if (useGamepadPrompt) tex = TextureManager::GetTexture(ftuePushPad);
		else tex = TextureManager::GetTexture(ftuePush);
		break;
	}
	}

	if (tex != nullptr)
		ftueQueue.emplace(DungeonGameFTUE(tex, type));
	else
		LogUtils::Log("Unable to locate FTUE Prompt.");
}

bool Crawl::DungeonGameManager::IsFTUECompleted(DungeonGameFTUE::FTUEType type)
{
	switch (type)
	{
	case DungeonGameFTUE::FTUEType::Turn:
	{
		if (player->GetState() == DungeonPlayer::STATE::TURNING) return true;
		break;
	}
	case DungeonGameFTUE::FTUEType::Move:
	{
		if (player->GetState() == DungeonPlayer::STATE::MOVING) return true;
		break;
	}
	case DungeonGameFTUE::FTUEType::Interact:
	{
		if (player->ftueHasInteracted) return true;
		break;
	}
	case DungeonGameFTUE::FTUEType::Look:
	{
		if (player->ftueHasLooked) return true;
		break;
	}
	case DungeonGameFTUE::FTUEType::Wait:
	{
		if (player->GetState() == DungeonPlayer::STATE::WAIT) return true;
		break;
		break;
	}
	case DungeonGameFTUE::FTUEType::Reset:
	{
		// not ever required
		break;
	}
	case DungeonGameFTUE::FTUEType::Push:
	{
		if (player->ftueHasPushed) return true;
		break;
	}
	default:
		return false;
	}

	return false;
}

void Crawl::DungeonGameManager::UpdateFTUE(float delta)
{
	if (ftueQueue.size() > 0)
	{

		if (!ftueIsCompleting && IsFTUECompleted(ftueQueue.front().type)) ftueIsCompleting = true;
		// Process queue!
		if (ftueIsCompleting)
		{
			ftueFadeTimeCurrent = max(ftueFadeTimeCurrent - delta, 0.0f);
			if (ftueFadeTimeCurrent == 0.0f)
			{
				ftueIsCompleting = false;
				ftueQueue.pop();
			}
		}
		else
		{
			ftueFadeTimeCurrent = min(ftueFadeTimeCurrent + delta, ftueFadeTime);

		}

		float alpha = ftueFadeTimeCurrent / ftueFadeTime;
		Texture* texture = ftueQueue.front().texture;

		glm::vec2 size = Window::GetViewPortSize();
		glm::vec2 pos = { size.x / 2, size.y * 0.9f };
		glm::vec2 texSize = { texture->width, texture->height };
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::SetNextWindowPos({ pos.x, pos.y }, ImGuiCond_Always, { 0.5,0.5f });
		ImGui::SetNextWindowSize({ texSize.x, texSize.y });
		ImGui::Begin("FTUE", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::Image(
			(ImTextureID)texture->texID,
			{ texSize.x, texSize.y },
			{ 0,1 },
			{ 1,0 },
			{ 1,1,1, alpha });
		ImGui::PopStyleVar();
		ImGui::End();
	}
}

void Crawl::DungeonGameManager::ClearAllFTUE()
{
	while (!ftueQueue.empty()) ftueQueue.pop();
	ftueFadeTimeCurrent = 0.0f;
	ftueIsCompleting = false;
}

void Crawl::DungeonGameManager::DoFTUEEvent(FTUEEvent event)
{
	switch (event)
	{
	case FTUEEvent::Turn:
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Turn);
		break;
	}
	case FTUEEvent::Move:
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Move);
		break;
	}
	case FTUEEvent::Look:
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Look);
		break;
	}
	case FTUEEvent::Reset:
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Reset);
		break;
	}
	case FTUEEvent::Wait:
	{
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Wait);
		break;
	}

	}
}

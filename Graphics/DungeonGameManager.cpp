#include "DungeonGameManager.h"
#include "DungeonGameManagerEvent.h"
#include "DungeonPlayer.h"
#include "DungeonDoor.h"
#include "DungeonLight.h"
#include "Scene.h"
#include "MathUtils.h"
#include "gtx/easing.hpp"

#include "LogUtils.h"
#include "AudioManager.h"

Crawl::DungeonGameManager* Crawl::DungeonGameManager::instance = nullptr;

Crawl::DungeonGameManager::DungeonGameManager()
{
	instance = this;
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
	if (player->GetDungeonLoaded()->isLobby)
		UpdateLobbyVisuals(delta);
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
	if (frontDoorLocksSceneObject)
		frontDoorLocksSceneObject->markedForDeletion = true;

	frontDoorLocksSceneObject = nullptr;
	frontDoorLeft = nullptr;
	frontDoorRight = nullptr;
	for (int i = 0; i < 4; i++) frontDoorLocksLatches[i] = nullptr;
}

void Crawl::DungeonGameManager::ConfigureLobby()
{
	Dungeon* lobby1 = player->GetDungeonLoaded();
	Dungeon* lobby2 = player->GetDungeonLobbyLevel2();
	
	// Get Light References
	lobbyHintLight = lobby1->GetLightWithID(lobbyHintLightID);
	lobbyLightingLight = lobby1->GetLightWithID(lobbyLightningLightID);
	for (int i = 0; i < 8; i++)
	{
		doorLights[i] = lobby1->GetLightWithID(i + 1);
	}

	// Get Door references
	doors[0] = lobby1->GetDoorWithID(3);
	doors[1] = lobby1->GetDoorWithID(2);
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

			if (enabledLights[i] && doorLights[i])
				doorLights[i]->Enable();
			else if (!enabledLights[i] && doorLights[i])
			{
				doorLights[i]->Disable();
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



		//if (frontDoorUnlocked[i]) frontDoorLocksSceneObject->children[i]->children[0]->markedForDeletion = true;
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
		player->ClearFTUEPrompt();
		return;
	}
	case 3: // trigger Move FTUE
	{
		player->SetFTUEPrompt(promptMove);
		return;
	}

	case 4: // trigger look prompt
	{
		if (!player->ftueHasLooked) player->SetFTUEPrompt(promptLook);
		return;
	}
	case 5: // trigger Interact Prompt
	{
		player->SetFTUEPrompt(promptInteract);
		return;
	}
	case 6: // Trigger Wait prompt
	{
		player->SetFTUEPrompt(promptWait);
		return;
	}
	case 7: // Trigger Reset prompt
	{
		player->SetFTUEPrompt(promptReset);
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
	// lightning strike
	// This gotta be moved to some game / global event manager
	if (lobbyHasTriggeredLightning && lobbyLightingLight != nullptr && lobbyLightningTimeCurrent < (lobbyLightningStrikeTime + 0.15f))
	{
		float t = glm::bounceEaseIn(glm::clamp(lobbyLightningTimeCurrent / lobbyLightningStrikeTime, 0.0f, 1.0f));
		t = glm::clamp(t, 0.0f, 1.0f);
		lobbyLightingLight->intensity = MathUtils::Lerp(0.0f, 1000.0f, t);
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
			lobbyLightingLight->intensity = 0.0f;
			lobbyLightingLight->UpdateLight();
		}
	}
}

void Crawl::DungeonGameManager::DoFTUEEvent(FTUEEvent event)
{
	switch (event)
	{
	case FTUEEvent::Turn:
	{
		player->SetFTUEPrompt(promptTurn);
		break;
	}
	case FTUEEvent::Move:
	{
		player->SetFTUEPrompt(promptMove);
		break;
	}
	case FTUEEvent::Look:
	{
		player->SetFTUEPrompt(promptLook);
		break;
	}
	case FTUEEvent::Reset:
	{
		player->SetFTUEPrompt(promptReset);
		break;
	}
	case FTUEEvent::Wait:
	{
		player->SetFTUEPrompt(promptWait);
		break;
	}

	}
}

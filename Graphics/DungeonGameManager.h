#pragma once
#include "serialisation.h"
#include <string>

class Object;

namespace Crawl
{
	class DungeonPlayer;
	class DungeonDoor;
	class DungeonLight;
	class DungeonGameManagerEvent;

	class DungeonGameManager
	{
	public:
		enum class FTUEEvent
		{
			Turn,
			Move,
			Look,
			Interact,
			Reset,
			Wait
		};
		enum class LobbyDoor // not used right now, all strings and IDs.
		{
			GroundNorth1,
			GroundSouth1,
			Level2North1,
			Level2North2,
			Level2North3,
			Level2South1,
			Level2South2,
			Level2South3
		};
		enum class DoorState
		{
			Open,
			Closed,
			Barricaded
		};

		static void Init();
		static DungeonGameManager* Get() { return instance; }

		bool DrawGUI();
		bool DrawGUIInternal();

		void Update(float delta);

		void SetPlayer(DungeonPlayer* player) { instance->player = player; }
		
		void RunGMEvent(const DungeonGameManagerEvent& gme);
		void OpenDoor(int doorIndex) { doorStates[doorIndex] = DoorState::Open; }
		void CloseDoor(int doorIndex) { doorStates[doorIndex] = DoorState::Closed; }
		void BarricadeDoor(int doorIndex) { doorStates[doorIndex] = DoorState::Barricaded; }
		void EnableLight(int lightIndex) { enabledLights[lightIndex] = true; }
		void DisableLight(int lightIndex) { enabledLights[lightIndex] = false; }

		void RemoveFrontDoorLock(int lockID) { frontDoorUnlocked[lockID-1] = true; }
		void ClearLocksObject();

		void ConfigureLobby();
		void ConfigureLobbyDoor();

		void DoEvent(int eventID);

		void ActivateLobbyLight() { lobbyHasTriggeredLightning = true; lobbyLightningTimeCurrent = 0.0f; }
		void UpdateLobbyVisuals(float delta);

		void DoFTUEEvent(FTUEEvent event);

	private:
		DungeonGameManager();
		static DungeonGameManager* instance;
		
		DungeonPlayer* player;

		// Lobby Configuration Items
		// Status
		bool manageLobby = false;
		DoorState doorStates[8] = { DoorState::Open, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed };
		bool enabledLights[8] = { true, false, false, false, false, false, false, false };
		bool lobbyHasTriggeredLightning = false;
		
		// References
		std::string doorNames[8] = { "G  North 1","G  South 1","L2 North 1", "L2 North 2", "L2 North 3","L2 South 1", "L2 South 2", "L2 South 3" };
		std::string doorStateNames[3] = { "Open", "Closed", "Barricaded" };
		DungeonDoor* doors[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		DungeonLight* doorLights[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };


		int lobbyHintLightID = 420;
		DungeonLight* lobbyHintLight = nullptr;

		int lobbyLightningLightID = 6969;
		DungeonLight* lobbyLightingLight = nullptr;
		string lightningSfx = "crawler/sound/load/lightning_strike.wav";
		bool playedSfx = false;
		float lobbyLightningTimeCurrent = 0.0f;
		const float lobbyLightningStrikeTime = 0.5f;

		// Front Door Locks
		bool frontDoorUnlocked[4] = { false, false, false, false };
		Object* frontDoorLocksSceneObject = nullptr;
		Object* frontDoorLeft = nullptr;
		Object* frontDoorRight = nullptr;
		Object* frontDoorLocksLatches[4] = { nullptr, nullptr, nullptr, nullptr };
		//float frontDoorLockZPositions[4] = {  }

		std::string frontDoorLocksObjectFilePath = "crawler/object/lobbyFrontDoorLocks.object";
		std::string frontDoorLocksLeftDoor = "crawler/model/door_frontdoor_left.object";
		std::string frontDoorLocksRightDoor = "crawler/model/door_frontdoor_right.object";
		std::string frontDoorLocksBracket = "crawler/model/door_bracket.object";
		std::string frontDoorLocksLatch = "crawler/model/door_latch.object";
		std::string frontDoorLocksPadEye = "crawler/model/door_pad_eye.object";

		float doorSwingAmount = 0.0f;


		// FTUE Configuration Items
		std::string promptTurn = "crawler/texture/gui/prompt_turn.tga";
		std::string promptMove = "crawler/texture/gui/prompt_move.tga";
		std::string promptLook = "crawler/texture/gui/prompt_look.tga";
		std::string promptInteract = "crawler/texture/gui/prompt_interact.tga";
		std::string promptReset = "crawler/texture/gui/prompt_reset.tga";
		std::string promptWait = "crawler/texture/gui/prompt_wait.tga";
	};
}


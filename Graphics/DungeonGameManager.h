#pragma once
#include "serialisation.h"
#include "DungeonGameFTUE.h"

#include <queue>
#include <string>

class Object;
class ComponentRenderer;
class Model;

namespace Crawl
{
	class DungeonPlayer;
	class DungeonEditor;
	class DungeonDoor;
	class DungeonLight;
	class DungeonGameManagerEvent;
	class DungeonMenu;
	class DungeonEnemyBlocker;

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
			ResetHold,
			Wait,
			Door,
			Box,
			Chaser,
			Key,
			FailedLevel
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

		enum class VoidState
		{
			HoldInDoor,
			LightsOut,
			End,
			Done
		};

		static void Init();
		static DungeonGameManager* Get() { return instance; }

		bool DrawGUI();
		bool DrawGUIInternal();

		void Update(float delta);

		void SetPlayer(DungeonPlayer* player) { instance->player = player; }
		void SetMenu(DungeonMenu* menu) { instance->menu = menu; }
		void SetEditor(DungeonEditor* editor) { instance->editor = editor; }
		DungeonMenu* GetMenu() { return menu; }

		void PauseGame();
		void UnpauseGame();
		void ReturnToEditor();
		void ResetGameState();
		
		void RunGMEvent(const DungeonGameManagerEvent& gme);
		void OpenDoor(int doorIndex) { doorStates[doorIndex] = DoorState::Open; }
		void CloseDoor(int doorIndex) { doorStates[doorIndex] = DoorState::Closed; }
		void BarricadeDoor(int doorIndex) { doorStates[doorIndex] = DoorState::Barricaded; }
		void EnableLight(int lightIndex) { enabledLights[lightIndex] = true; }
		void DisableLight(int lightIndex) { enabledLights[lightIndex] = false; }

		void RemoveFrontDoorLock(int lockID) { frontDoorUnlocked[lockID] = true; }
		void ClearLocksObject();

		void FindAllEmissiveWindows();
		void SetAllEmissiveWindows(float emissiveScale);

		void ConfigureLobby();
		void ConfigureLobbyDoor();
		void UpdateDoorStateEvent();
		void MakeLobbyExitTraversable();
		void MakeLobbyVoidUnTraversable();

		void DoEvent(int eventID);

		void ActivateLobbyLight() { lobbyHasTriggeredLightning = true; lobbyLightningTimeCurrent = 0.0f; }
		void UpdateLobbyVisuals(float delta);
		void UpdateLobbyVisualsLightning(float delta);
		void UpdateLobbyVisualsLocks(float delta);
		
		void UpdateVoidVisuals(float delta);

		void QueueFTUEPrompt(DungeonGameFTUE::FTUEType type);
		bool IsFTUECompleted(DungeonGameFTUE::FTUEType type);
		void UpdateFTUE(float delta);
		void CheckForBrokenLevel();
		void ClearAllFTUE();

		void DoFTUEEvent(FTUEEvent event);
		bool manageLobby = true;

	private:
		DungeonGameManager();
		static DungeonGameManager* instance;
		
		DungeonPlayer* player;
		DungeonMenu* menu;
		DungeonEditor* editor;

		// Lobby Configuration Items
		// Status
		DoorState doorStates[8] = { DoorState::Open, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed, DoorState::Closed };
		bool enabledLights[8] = { true, false, false, false, false, false, false, false };
		bool lobbyHasTriggeredLightning = false;
		
		// References
		std::string doorNames[8] = { "G  South 1", "G  North 1", "L2 North 1", "L2 North 2", "L2 North 3","L2 South 1", "L2 South 2", "L2 South 3" };
		std::string doorStateNames[3] = { "Open", "Closed", "Barricaded" };
		DungeonDoor* doors[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		DungeonLight* doorLights[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };


		// Lightening Strikes
		int lobbyLightningLightID = 6969;
		DungeonLight* lobbyLightingLight = nullptr;
		string lightningSfx = "crawler/sound/load/lightning_strike.wav";
		bool playedSfx = false;
		float lobbyLightningTimeCurrent = 0.0f;
		const float lobbyLightningStrikeTime = 0.5f;
		std::vector<Model*> lighteningAffectedModels; // List of model pointers that should have their emmissive scale controlled by the lighting strike
		std::vector<ComponentRenderer*> lighteningAffectedRenderers; // list of renderers as build by matching model pointers above;

		// Front Door lock hinges
		bool frontDoorUpdateTriggered = false;
		bool frontDoorUnlocked[4] = { false, false, false, false };
		bool frontDoorUnlockAnimationStarted[4] = { false, false, false, false };
		float frontDoorUnlockHingeT[4] = { 0,0,0,0 };
		float frontDoorHingePosStart = 1.5f;
		float frontDoorHingePosEnd = -178.0f;
		float frontDoorHingeOpenSpeed = 0.70f;
		float frontDoorHingeZPositions[4] = { 1.95f, 1.65, 1.35, 1.05 };

		Object* frontDoorLocksSceneObject = nullptr;
		Object* frontDoorLeft = nullptr;
		Object* frontDoorRight = nullptr;
		Object* frontDoorLocksLatches[4] = { nullptr, nullptr, nullptr, nullptr };

		// Front Door Lock PadLocks
		string frontDoorPadLockObjectPath = "crawler/object/lobbyFrontDoorPadLock.object";
		string frontDoorPadLockShacklePath = "crawler/model/door_frontdoor_padlock_shackle.object";
		string frontDoorPadLockPadlockPath = "crawler/model/door_frontdoor_padlock_lock.object";
		float frontDoorPadLockOpenAngle = -52.0f;
		float frontDoorPadLockOpenSpeed = 0.4f;
		float frontDoorPadLockFallSpeed = 0.3f;

		float frontDoorUnlockShackleT[4] = { 0,0,0,0 };
		float frontDoorUnlockPadlockT[4] = { 0,0,0,0 };
		float frontDoorPadlockZPositions[4] = { 0, 0, 0, 0 }; // generated in constructor based on hinge positions
		float frontDoorPadlockZOffset = -0.021f; // 0.215;
		Object* frontDoorPadLockObjects[4] = { nullptr, nullptr, nullptr, nullptr };
		Object* frontDoorPadLockShackleObjects[4] = { nullptr, nullptr, nullptr, nullptr };
		//Object* frontDoorPadLockLockObjects[4] = { nullptr, nullptr, nullptr, nullptr };

		// Front Door
		bool frontDoorOpenAnimationStarted = false;
		float frontDoorOpenT = 0.0f;
		float frontDoorOpenRotationStart = 90;
		float frontDoorOpenRotationEnd = 130;
		float frontDoorOpenSpeed = 10.0f;
		glm::ivec2 frontDoorUnlockPositionMin = { -2, 0 };
		glm::ivec2 frontDoorUnlockPositionMax = { -1, 0 };

		glm::ivec2 positionToMakeTraversable = { -2, 0 };
		glm::ivec2 positionToMakeUntraversable = { -3, 0 };

		std::string frontDoorLocksObjectFilePath = "crawler/object/lobbyFrontDoorLocks.object";
		std::string frontDoorLocksLeftDoor = "crawler/model/door_frontdoor_left.object";
		std::string frontDoorLocksRightDoor = "crawler/model/door_frontdoor_right.object";
		std::string frontDoorLocksBracket = "crawler/model/door_bracket.object";
		std::string frontDoorLocksLatch = "crawler/model/door_latch.object";
		std::string frontDoorLocksPadEye = "crawler/model/door_pad_eye.object";

		float doorSwingAmount = 0.0f; // this is debug for imgui

		// void ending sequence
		int voidLights = 1;
		bool startVoid = false;
		bool voidTrigger = false;
		float voidSoundTimer = 0.0f;
		float voidTriggerTimer = 0.0f;
		bool voidTriggerLightStarted = false;
		const float voidSoundTimeStart = 3.0f;
		float voidSoundTime = voidSoundTimeStart;
		const float voudSoundTimeSpeedUp = 0.333333f;
		const float voidEnterTime = 3.0f;
		const float voidLightsOutTime = 2.5f;
		const float voidLightsOnStartNegativeDelta = 1.5f;
		const float voidEndTime = .45f;
		DungeonEnemyBlocker* voidBlocker = nullptr;
		VoidState voidState = VoidState::HoldInDoor;

		// FTUE Configuration Items
		std::queue<DungeonGameFTUE> ftueQueue;
		std::vector<DungeonGameFTUE::FTUEType> ftueCompleted;
		bool ftueIsCompleting = false;
		float ftueFadeTimeCurrent = 0.0f;
		const float ftueFadeTime = 0.75;
		float ftueAutoCompleteCurrent = 0.0f;
		const float ftueAutoCompleteTime = 2.0f;

		std::string ftueTurn = "crawler/texture/gui/ftue/turn.tga";
		std::string ftueTurnPad = "crawler/texture/gui/ftue/turnPad.tga";
		std::string ftueMove = "crawler/texture/gui/ftue/move.tga";
		std::string ftueMovePad = "crawler/texture/gui/ftue/movePad.tga";
		std::string ftueLook = "crawler/texture/gui/ftue/look.tga";
		std::string ftueLookPad = "crawler/texture/gui/ftue/lookPad.tga";
		std::string ftueInteract = "crawler/texture/gui/ftue/interact.tga";
		std::string ftueInteractPad = "crawler/texture/gui/ftue/interactPad.tga";
		std::string ftueReset = "crawler/texture/gui/ftue/reset.tga";
		std::string ftueResetPad = "crawler/texture/gui/ftue/resetPad.tga";
		std::string ftueResetHold = "crawler/texture/gui/ftue/resetHold.tga";
		std::string ftueResetHoldPad = "crawler/texture/gui/ftue/resetHoldPad.tga";
		std::string ftueWait = "crawler/texture/gui/ftue/wait.tga";
		std::string ftueWaitPad = "crawler/texture/gui/ftue/waitPad.tga";
		std::string ftuePush = "crawler/texture/gui/ftue/push.tga";
		std::string ftuePushPad = "crawler/texture/gui/ftue/pushPad.tga";

		// New ones
		std::string ftueDoor1 = "crawler/texture/gui/ftue/door_1.tga";
		std::string ftueDoor2 = "crawler/texture/gui/ftue/door_2.tga";
		std::string ftueRun = "crawler/texture/gui/ftue/run.tga";
		std::string ftueKey = "crawler/texture/gui/ftue/keys.tga";
	};
}


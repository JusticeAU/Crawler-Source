#pragma once
#include "Dungeon.h"
#include "DungeonHelpers.h"
#include "glm.hpp"

class Object;
class ComponentAnimator;
class ComponentCamera;
class ComponentLightPoint;
class ComponentRenderer;
class Texture;

namespace Crawl
{
	class DungeonMenu;
	class DungeonPlayer
	{
	public:
		enum class PlayerCommand
		{
			None,
			Forward,
			Back,
			Left,
			Right,
			TurnLeft,
			TurnRight,
			Interact,
			Wait,
			Reset
		};
		enum STATE {
			NOSTATE,
			MENU,
			IDLE,
			WAIT,
			MOVING,
			TURNING,
			STAIRBEARS,
			DYING,
			TRANSPORTER,
			NOCONTROL
		};

		enum class RHState
		{
			Idle,
			DownIdle,
			WalkForward,
			WalkBack,
			WalkLeft,
			WalkRight,
			DownWalk,
			MoveDown,
			MoveUp,
			Stairs
		};
		DungeonPlayer();

		void SetDungeon(Dungeon* dungeonPtr);
		Dungeon* GetDungeonLoaded() { return dungeon; }
		Dungeon* GetDungeonLobbyLevel2() { return lobbyLevel2Dungeon; }

		void SetLightIntensity(float intensity);
		void SetLanternEmissionScale(float emissionScale);

		STATE GetState() { return state; }
		void SetNextState(STATE state = STATE::IDLE) { this->nextState = state; }
		void SetStateIdle() { state = IDLE; }

		void SetMenu(DungeonMenu* menu) { gameMenu = menu; }

		bool Update(float deltaTime);
		void UpdatePointOfInterestTilt(bool instant = false);
		void ContinueResettingTilt(float delta);

		ivec2 GetPosition() { return position; }
		ivec2 GetPositionPrevious() { return positionPrevious; }

		FACING_INDEX GetOrientation() { return facing; }
		
		void SetShouldActivateTransporter(DungeonTransporter* transporter);
		void LoadSelectedTransporter(DungeonTransporter* transporter);
		void Teleport(ivec2 position);
		void Orient(FACING_INDEX facing);
		void ResetPlayer();
		void ReturnToLobby();
		
		void SetRespawn(ivec2 position, FACING_INDEX orientation, bool isLevel2 = false);
		void ClearRespawn();
		void Respawn();

		void MakeCheckpoint(FACING_INDEX direction);
		void ClearCheckpoint();

		void TakeDamage();

		void SetShouldSwitchWith(DungeonEnemySwitcher* switcher) { shouldSwitchWith = switcher; }
		void SetShouldActivateStairs(DungeonStairs* stairs);

		// combines our requested direction with our facing direction to return the actual direction.
		unsigned int GetMoveCardinalIndex(DIRECTION_INDEX dir);

		void SetLevel2(bool level2);

		void DoEvent(int eventID);
		bool PostUpdateComplete = false;
		
		PlayerCommand inputBuffer = PlayerCommand::None;

		bool didJustRespawn = false;
		bool useRespawnSound = true;
		bool canResetOrWait = true;
		bool enableDebugUI = true;
		bool usingLevelEditor = false;
		// Accessability Options
		bool autoReOrientDuringFreeLook = true;
		bool alwaysFreeLook = false;
		bool invertYAxis = false;
	private:
		void UpdateInputs();
		void UpdateInputsMovement();
		void UpdateInputsLooking();

		bool UpdateFreeLook(float delta, bool dontAutoReorient = false);
		void SetFreeLookReset();
		void HandleLookTilt(float delta);
		bool IsLookingWithGamepad();

		bool UpdateStateIdle(float delta);
		bool IsMoveDown();
		bool IsMovePressedLongEnough(float delta);
		
		int GetMoveDirection();
		int GetMoveIndex();
		void TurnLeft(bool autoReorient = false);
		void TurnRight(bool autoReorient = false);

		bool UpdateStateMoving(float delta);
		bool UpdateStateTurning(float delta);
		bool UpdateStateStairs(float delta);
		bool UpdateStateDying(float delta);
		void UpdateStateTransporter(float delta);
		
		void SetStateRH(RHState state);
		void UpdateStateRH(float delta);
		void DrawDevelopmentBuildUI();

		void SoundPlayFootstep();
		void DrawCrosshair();

		glm::ivec2 position = {0,0};
		glm::ivec2 positionPrevious = { 0,0 };
		FACING_INDEX facing = EAST_INDEX;
		
		STATE state = IDLE;
		STATE nextState = NOSTATE;
		float lookSpeed = 0.1f;
		float lookMaxX = 45.0f;
		float lookMaxZ = 170.0f;
		const float lookRestXDefault = -7.0f;
		const float lookRestXInterest = -17.0f;
		float lookRestX = lookRestXDefault;
		bool isFreeLooking = false;
		bool wasFreeLooking = false;
		bool hasLookedWithGamepad = false;
		glm::vec3 lookReturnFrom = { 0,0,0 };
		bool wasLookingAtPointOfInterest = false;
		float lookReturnTimeTotal = 0.25f;
		float lookReturnTimeCurrent = 1.0f;

		float moveSpeed = 0.25f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition = { 0,0,0 };
		glm::vec3 targetPosition = { 0,0,0 };

		float moveDelay = 0.0f;
		float moveDelayCurrent = 0.0f;

		bool turnCurrentIsManual = true;
		float turnSpeedManual = 0.25f;
		float turnSpeedReOrient = 0.35;
		float turnCurrent = 0.0f;
		float turnDeltaPrevious = 0.0f;
		float oldTurn = 0.0f;
		float targetTurn = 0.0f;
		
		Dungeon* dungeon = nullptr;	// This is a pointer to the currently loaded dungeon.
							// The player stores the level level 2 in memory, and currentDungeon will either be this pointer, or lobbyLevel2.
		Object* object = nullptr;
		Object* objectView = nullptr;
		ComponentAnimator* animator = nullptr;
		ComponentCamera* camera = nullptr;
		ComponentLightPoint* light = nullptr;
		ComponentRenderer* renderer = nullptr;

		bool didMove = false;

		int maxHp = 1;
		int hp = maxHp;

		bool hasRespawnLocation = false;
		glm::ivec2 respawnPosition = { 0,0 };
		FACING_INDEX respawnOrientation = EAST_INDEX;
		bool respawnLevel2 = false;

		DungeonEnemySwitcher* shouldSwitchWith = nullptr;
		DungeonStairs* activateStairs = nullptr;

		// Checkpointing
	public:
		bool checkpointExists = false;
		ordered_json checkpointSerialised;
		glm::ivec2 checkpointPosition = { 0,0 };
		FACING_INDEX checkpointFacing = NORTH_INDEX;
	
	protected:
		string stepSounds[4] = {
			"crawler/sound/load/step1.ogg",
			"crawler/sound/load/step2.ogg",
			"crawler/sound/load/step3.ogg",
			"crawler/sound/load/step4.ogg"
		};
		int lastStepIndex = 0;

		// Lobby Scene Stuff
	public:
		bool isOnLobbyLevel2 = false;
		Dungeon* lobbyLevel2Dungeon = nullptr;
		Dungeon* currentDungeon = nullptr; // This just changes between dungeon and lobbyLevel2Dungeon. dungeon stores the actual dungeon we have loaded.
		
		float lobbyLevel2Floor = 3.2f;
		float stairTimeTotal = 3.0f;
		float stairTimeCurrent = 0.0f;
		float playerZPosition = 0.0f;
		bool facingTarget = false;


		// Fading stuff
		bool fadeIn = true;
		float fadeTimeCurrent = 0.0f;
		float fadeTimeTotal = 0.7f;

		glm::vec3 deathColour = glm::vec3(0.121, 0.0, 0.0); // very dark red.
		enum class KILLEDBY
		{
			SPIKES,
			CHASER,
			SAWARINA,
			LASER
		};
		KILLEDBY killedBy = KILLEDBY::SPIKES;
		FACING_INDEX killedFrom = FACING_INDEX::NORTH_INDEX;

		// FTUE - Move in to Game Manager once created.
		bool ftueEnabled = false;
		bool ftueInitiated = false;
		string promptCurrent = "";
		string promptNext = "";

		int ftueTurns = 0;
		bool ftueHasTurned = false;
		bool ftueHasLooked = false;
		bool ftueHasInteracted = false;
		bool ftueHasWaited = false;
		bool ftueHasPushed = false;

		glm::vec3 transporterColour = glm::vec3(0.0, 0.0, 0.0);
		DungeonTransporter* transporterToActivate = nullptr;

		DungeonMenu* gameMenu = nullptr;

		// viewmodel stuff
		// Right hand
		bool rhShouldBeDown = false;
		bool rhIsDown = false;
		RHState stateRH = RHState::Idle;
		string animationRHBaseName = "crawler/model/viewmodel.fbxviewmodel_rig|";

		string animationRHIdle = animationRHBaseName + "idle";
		string animationRHWalkForward = animationRHBaseName + "grid_move_forward";
		string animationRHWalkBack = animationRHBaseName + "grid_move_back";
		string animationRHWalkLeft = animationRHBaseName + "grid_move_left";
		string animationRHWalkRight = animationRHBaseName + "grid_move_right";

		string animationRHDown = animationRHBaseName + "down";
		string animationRHDownIdle = animationRHBaseName + "down_idle";
		string animationRHDownWalk = animationRHBaseName + "down_walk";
		string animationRHUp = animationRHBaseName + "down_reverse";
		string animationRHStairs = animationRHBaseName + "walk_stairs";

		string animationNamePush = "crawler/model/viewmodel_hands.fbxarmature|armatureaction";

		// Accessability crosshair
		const string crossHairPath = "crawler/texture/gui/crosshair.tga";
		Texture* crosshairTexture = nullptr;
		bool showCrosshair = false;
	};
}

#pragma once
#include "Dungeon.h"
#include "DungeonHelpers.h"
#include "glm.hpp"

class Object;
class ComponentAnimator;
class ComponentCamera;


namespace Crawl
{
	class DungeonMenu;
	class DungeonPlayer
	{
	public:

		enum STATE {
			MENU,
			IDLE,
			WAIT,
			MOVING,
			TURNING,
			STAIRBEARS,
			DYING,
			TRANSPORTER
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

		void SetStateIdle() { state = IDLE; }

		void SetMenu(DungeonMenu* menu) { gameMenu = menu; }

		bool Update(float deltaTime);
		void UpdatePointOfInterestTilt(bool instant = false);
		void ContinueResettingTilt(float delta);

		ivec2 GetPosition() { return position; }
		ivec2 GetPositionPrevious() { return positionPrevious; }

		FACING_INDEX GetOrientation() { return facing; }
		
		void SetShouldActivateTransporter(DungeonTransporter* transporter);
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
		
		bool didJustRespawn = false;

		bool enableDebugUI = true;

		// Accessability Options
		bool autoReOrientDuringFreeLook = true;
		bool alwaysFreeLook = false;
		bool invertYAxis = false;
	private:
		bool HandleFreeLook(float delta);
		void HandleLookTilt(float delta);

		bool UpdateStateIdle(float delta);
		bool IsMoveDown();
		bool IsMovePressedLongEnough(float delta);
		
		int GetMoveDirection();
		int GetMoveIndex();
		void TurnLeft(bool updateFreeLook = false);
		void TurnRight(bool updateFreeLook = false);

		bool UpdateStateMoving(float delta);
		bool UpdateStateTurning(float delta);
		bool UpdateStateStairs(float delta);
		bool UpdateStateDying(float delta);
		void UpdateStateTransporter(float delta);
		
		void SetStateRH(RHState state);
		void UpdateStateRH(float delta);

		void LoadSelectedTransporter(DungeonTransporter* transporter);

	public:
		void UpdatePrompts(float delta);
		void UpdateFTUE();
		void SetFTUEPrompt(string prompt);
		void ClearFTUEPrompt(bool instant = false);
	private:
		void DrawDevelopmentBuildUI();

		void SoundPlayFootstep();

		glm::ivec2 position;
		glm::ivec2 positionPrevious;
		FACING_INDEX facing = EAST_INDEX;
		
		STATE state = IDLE;
		float lookSpeed = 0.1f;
		float lookMaxX = 45.0f;
		float lookMaxZ = 170.0f;
		const float lookRestXDefault = -7.0f;
		const float lookRestXInterest = -17.0f;
		float lookRestX = lookRestXDefault;
		glm::vec3 lookReturnFrom;
		bool wasLookingAtPointOfInterest = false;
		float lookReturnTimeTotal = 0.25f;
		float lookReturnTimeCurrent = 1.0f;

		float moveSpeed = 0.25f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition;
		glm::vec3 targetPosition;

		float moveDelay = 0.0f;
		float moveDelayCurrent = 0.0f;

		float turnSpeed = 0.25f;
		float turnCurrent = 0.0f;
		bool turnShouldApplyDeltaToFreeLook = false;
		float turnDeltaPrevious = 0.0f;
		float oldTurn;
		float targetTurn;
		
		Dungeon* dungeon;	// This is a pointer to the currently loaded dungeon.
							// The player stores the level level 2 in memory, and currentDungeon will either be this pointer, or lobbyLevel2.
		Object* object;
		Object* objectView;
		ComponentAnimator* animator;
		ComponentCamera* camera;

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

		bool promptFadeIn = false;
		float promptAmount = 0.0f;
		const float promptFadeTime = 0.4f;
		bool promptUse = false;

		glm::vec3 transporterColour = glm::vec3(0.0, 0.0, 0.0);
		DungeonTransporter* transporterToActivate = nullptr;

		DungeonMenu* gameMenu = nullptr;

		// viewmodel stuff
		// Right hand
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
	};
}

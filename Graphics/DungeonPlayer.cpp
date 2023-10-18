#include "DungeonPlayer.h"
#include "DungeonGameManager.h"
#include "Input.h"
#include "Window.h"
#include "Scene.h"
#include "MathUtils.h"
#include "LogUtils.h"
#include "AudioManager.h"
#include "ComponentAnimator.h"
#include "ComponentCamera.h"

#include "DungeonEnemySwitcher.h"
#include "DungeonCheckpoint.h"
#include "DungeonStairs.h"
#include "DungeonLight.h"
#include "DungeonTransporter.h"

#include "DungeonMenu.h"

#include "gtx/spline.hpp"
#include "gtx/easing.hpp"

Crawl::DungeonPlayer::DungeonPlayer()
{
	// Initialise the player Scene Object;
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk("crawler/object/player.object"));
	object->children[0]->children[0]->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/viewmodel.object"));
	animator = (ComponentAnimator*)object->children[0]->children[0]->children[0]->GetComponent(Component_Animator);
	animator->StartAnimation(animationRHIdle, true);
	objectView = object->children[0];
	camera = (ComponentCamera*)object->children[0]->GetComponent(Component_Camera);

	// Create the lobby second level
	lobbyLevel2Dungeon = new Dungeon(true);
	//lobbyLevel2Dungeon->Load("crawler/dungeon/lobby2.dungeon");
	lobbyLevel2Dungeon->player = this;
}

void Crawl::DungeonPlayer::SetDungeon(Dungeon* dungeonPtr)
{
	dungeon = dungeonPtr;
	currentDungeon = dungeon;
}

// Returns true if the player made a game-state changing action
bool Crawl::DungeonPlayer::Update(float deltaTime)
{
	//animator->DrawGUI();

	if(state != MENU)	HandleFreeLook(deltaTime);

	if(enableDebugUI) DrawDebugUI();

	if (ftueEnabled)
	{
		UpdateFTUE();
		UpdatePrompts(deltaTime);
	}

	UpdateStateRH(deltaTime);

	if (state == MENU)
	{
		gameMenu->DrawPauseMenu();
	}
	else if (state == WAIT)
	{
		moveCurrent += deltaTime;
		if (moveCurrent >= moveSpeed)
		{
			state = IDLE;
			moveCurrent = 0.0f;
		}
	}
	else if (state == IDLE)
	{
		if (UpdateStateIdle(deltaTime))
			return true;
	}
	else if (state == MOVING)
	{
		UpdateStateMoving(deltaTime);
	}
	else if (state == TURNING)
	{
		UpdateStateTurning(deltaTime);
	}
	else if (state == STAIRBEARS)
	{
		UpdateStateStairs(deltaTime);
	}
	else if (state == DYING)
	{
		UpdateStateDying(deltaTime);
		ContinueResettingTilt(deltaTime);
	}
	else if (state == TRANSPORTER)
	{
		UpdateStateTransporter(deltaTime);
		ContinueResettingTilt(deltaTime);
	}
	return false;
}

bool Crawl::DungeonPlayer::HandleFreeLook(float delta)
{
	if (Input::Mouse(1).Pressed() || alwaysFreeLook)
	{
		/*if (!ftueHasLooked && promptCurrent == promptLook);
		{
			ftueHasLooked = true;
			ClearFTUEPrompt();
		}*/
		vec2 mouseDelta = -Input::GetMouseDelta() * lookSpeed;
		if (invertYAxis) mouseDelta.y = -mouseDelta.y;
		objectView->AddLocalRotation({ mouseDelta.y, 0, mouseDelta.x });
		objectView->localRotation.x = glm::clamp(objectView->localRotation.x, -lookMaxX, lookMaxX);
		objectView->localRotation.z = glm::clamp(objectView->localRotation.z, -lookMaxZ, lookMaxZ);

		if (!autoReOrientDuringFreeLook && !alwaysFreeLook) return false;

		if (objectView->localRotation.z > 60 && state == IDLE)
		{
			TurnLeft(true);
			return false;
		}
		else if (objectView->localRotation.z < -60 && state == IDLE)
		{
			TurnRight(true);
			return false;
		}

	}

	if (Input::Mouse(1).Up())
	{
		lookReturnFrom = objectView->localRotation;
		lookReturnTimeCurrent = 0.0f;
	}

	return false;
}

void Crawl::DungeonPlayer::HandleLookTilt(float delta)
{
	if (alwaysFreeLook) return; // This doesnt run if we're we have freelook always on
	if(Input::Mouse(1).Pressed()) return; // or if we're currently freelookin.
	if (lookReturnTimeCurrent >= lookReturnTimeTotal) return; // We're done resetting

	ContinueResettingTilt(delta);
}

bool Crawl::DungeonPlayer::UpdateStateIdle(float delta)
{
	if (PostUpdateComplete)
	{
		PostUpdateComplete = false;
		dungeon->PostUpdate();
	}

	if (Input::Keyboard(GLFW_KEY_ESCAPE).Down() && gameMenu) // only perform this action if the gameMenu is initialised. This wont be the case in designer mode.
	{
		Window::GetWindow()->SetMouseCursorHidden(false);
		state = MENU;
		return false;
	}

	// All these checks should move to a turn processors state machine.
	if (hp <= 0)
	{
		LogUtils::Log("Died. Resetting & Respawning");
		state = DYING;
		camera->postProcessFadeColour = deathColour;
		fadeTimeCurrent = 0.0f;
		fadeIn = false;
		
		DungeonGameManager::Get()->DoFTUEEvent(DungeonGameManager::FTUEEvent::Reset);
		return false;
	}

	HandleLookTilt(delta);

	if (Input::Keyboard(GLFW_KEY_SPACE).Down() && currentDungeon->playerCanKickBox)
	{
		if (currentDungeon->DoKick(position, facing))
		{
			//animator->BlendToAnimation(animationNamePush, 0.1f);
			return true;
		}
	}

	if (Input::Keyboard(GLFW_KEY_R).Down())
	{
		if (isOnLobbyLevel2) lobbyLevel2Dungeon->GetTile(position)->occupied = false; // because this level doesnt reset, we need to keep it tidy!!
		Respawn();
	}

	if (currentDungeon->GetCheckpointAt(position))
	{
		DungeonCheckpoint* newCheckpoint = dungeon->GetCheckpointAt(position);
		if (!newCheckpoint->activated)
		{
			LogUtils::Log("You activated a checkpoint");
			newCheckpoint->SetCheckpoint(this);
		}
	}

	if (shouldSwitchWith) // fairly hacky
	{
		AudioManager::PlaySound("crawler/sound/load/switcher.wav");
		shouldSwitchWith->SwapWithPlayer();
		shouldSwitchWith = nullptr;
	}

	// To here.
	glm::ivec2 coordinate = { 0, 0 };
	glm::ivec2 coordinateUnchanged = { 0, 0 }; // TO DO this sucks

	if (Input::Keyboard(GLFW_KEY_LEFT_ALT).Down())
	{
		state = WAIT;
		moveCurrent = 0.0f;
		return true;
	}

	// Old Stab mechanic
	/*if ((Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Down() || Input::Mouse(1).Down()) && dungeon->playerHasKnife)
	{
		if (dungeon->CanSee(position, facing))
		{
			LogUtils::Log("Stab!");
			dungeon->DamageAtPosition(position + directions[facing], this, true);
			return true;
		}
	}*/

	// Activating Objects
	// Via Mouse
	if (Input::Mouse(0).Down())
		Scene::RequestObjectSelection();

	if (Scene::s_instance->objectPickedID != 0)
	{
		unsigned int picked = Scene::s_instance->objectPickedID;
		Scene::s_instance->objectPickedID = 0;
		if (currentDungeon->DoInteractable(picked))
		{
			animator->BlendToAnimation(animationNamePush, 0.1f);
			if (!currentDungeon->playerInteractIsFree)
				return true;
		}
	}
	
	// Via Interact Key
	if (Input::Keyboard(GLFW_KEY_SPACE).Down())
	{
		if (currentDungeon->DoInteractable(position, facing))
		{
			if (!ftueHasInteracted)
			{
				ftueHasInteracted = true;
				ClearFTUEPrompt();
			}
			UpdatePointOfInterestTilt();
			//animator->BlendToAnimation(animationNamePush, 0.1f);
			if (!currentDungeon->playerInteractIsFree)
				return true;
		}
	}

	ivec2 oldPlayerCoordinate = position;
	if (IsMoveDown() || IsMovePressedLongEnough(delta))
	{
		int index = GetMoveIndex();

		if (index != -1)
		{
			// Check stairs in this direction
			if (currentDungeon->ShouldActivateStairs(position, (FACING_INDEX)index))
			{
				DungeonTile* oldTile = currentDungeon->GetTile(position);
				if (oldTile) oldTile->occupied = false;
				state = STAIRBEARS;
				position = activateStairs->endPosition;
				stairTimeCurrent = 0.0f;
				SetStateRH(RHState::Stairs);
				if (activateStairs->up)
				{
					isOnLobbyLevel2 = true;
					currentDungeon = lobbyLevel2Dungeon;
					playerZPosition = lobbyLevel2Floor;
				}
				else
				{
					isOnLobbyLevel2 = false;
					currentDungeon = dungeon;
					playerZPosition = 0.0f;
				}
				return false;
			}
			else if (currentDungeon->PlayerCanMove(position, index))
			{
				// Check for Transporter there!
				if (currentDungeon->ShouldActivateTransporter(position, (FACING_INDEX)index))
				{
					if (index != facing)
					{
						facingTarget = false;
						turnCurrent = 0.0f;
						oldTurn = object->localRotation.z;
						targetTurn = orientationEulers[index];
					}
					else facingTarget = true;
					currentDungeon->GetTile(oldPlayerCoordinate)->occupied = false;
					return false;
				}
				if (!wasLookingAtPointOfInterest)
				{
					switch (GetMoveDirection())
					{
					case DIRECTION_INDEX::FORWARD_INDEX:
						SetStateRH(RHState::WalkForward);
						break;
					case DIRECTION_INDEX::BACK_INDEX:
						SetStateRH(RHState::WalkBack);
						break;
					case DIRECTION_INDEX::LEFT_INDEX:
						SetStateRH(RHState::WalkLeft);
						break;
					case DIRECTION_INDEX::RIGHT_INDEX:
						SetStateRH(RHState::WalkRight);
						break;
					}
				}
				SoundPlayFootstep();
				positionPrevious = position;
				position += directions[index];
				currentDungeon->GetTile(position)->occupied = true;
				didMove = true;
			}
		}
	}

	if (didMove)
	{
		currentDungeon->GetTile(oldPlayerCoordinate)->occupied = false;
		state = MOVING;
		oldPosition = object->localPosition;
		targetPosition = { position.x * Crawl::DUNGEON_GRID_SCALE, position.y * Crawl::DUNGEON_GRID_SCALE , playerZPosition };
		moveCurrent = 0.0f;
		didMove = false;
		
		UpdatePointOfInterestTilt();
		return true;
	}

	// Turning
	if (Input::Keyboard(GLFW_KEY_E).Down())
	{
		TurnRight();
		return false;
	}
	if (Input::Keyboard(GLFW_KEY_Q).Down())
	{
		TurnLeft();
		return false;
	}

	// Rotators - not in use
	/*if (Input::Keyboard(GLFW_KEY_Z).Down())
	{
		DungeonMirror* mirror = currentDungeon->GetMirrorAt(position + directions[facing]);
		if (mirror)
		{
			dungeon->RotateMirror(mirror, 1);
			return true;
		}
	}
	if (Input::Keyboard(GLFW_KEY_C).Down())
	{
		DungeonMirror* mirror = currentDungeon->GetMirrorAt(position + directions[facing]);
		if (mirror)
		{
			currentDungeon->RotateMirror(mirror, -1);
			return true;
		}
	}*/

	return false;
}

bool Crawl::DungeonPlayer::IsMoveDown()
{
	return
		Input::Keyboard(GLFW_KEY_W).Down() ||
		Input::Keyboard(GLFW_KEY_S).Down() ||
		Input::Keyboard(GLFW_KEY_A).Down() ||
		Input::Keyboard(GLFW_KEY_D).Down();
}

bool Crawl::DungeonPlayer::IsMovePressedLongEnough(float delta)
{
	if (Input::Keyboard(GLFW_KEY_W).Pressed() ||
		Input::Keyboard(GLFW_KEY_S).Pressed() ||
		Input::Keyboard(GLFW_KEY_A).Pressed() ||
		Input::Keyboard(GLFW_KEY_D).Pressed())
	{
		moveDelayCurrent += delta;
		if (moveDelayCurrent > moveDelay)
		{
			moveDelayCurrent = 0.0f;
			return true;
		}
	}
	else moveDelayCurrent = 0.0f;

	return false;
}

int Crawl::DungeonPlayer::GetMoveDirection()
{
	if (Input::Keyboard(GLFW_KEY_W).Pressed()) return FORWARD_INDEX;
	if (Input::Keyboard(GLFW_KEY_S).Pressed()) return BACK_INDEX;
	if (Input::Keyboard(GLFW_KEY_A).Pressed()) return LEFT_INDEX;
	if (Input::Keyboard(GLFW_KEY_D).Pressed()) return RIGHT_INDEX;

	return -1;
}

int Crawl::DungeonPlayer::GetMoveIndex()
{
	return GetMoveCardinalIndex((DIRECTION_INDEX)GetMoveDirection());
}

void Crawl::DungeonPlayer::TurnLeft(bool updateFreeLook)
{
	turnShouldApplyDeltaToFreeLook = updateFreeLook;
	AudioManager::PlaySound("crawler/sound/load/turn.wav");
	int faceInt = (int)facing;
	faceInt--;
	if (faceInt == -1)
		faceInt = 3;
	facing = (FACING_INDEX)faceInt;
	state = TURNING;
	turnCurrent = 0.0f;
	oldTurn = object->localRotation.z;
	targetTurn = object->localRotation.z + 90;
	UpdatePointOfInterestTilt();

	if (!ftueHasTurned) ftueTurns++;
	dungeon->DoEventTriggerFacing(position, facing);
}

void Crawl::DungeonPlayer::TurnRight(bool updateFreeLook)
{
	turnShouldApplyDeltaToFreeLook = updateFreeLook;
	AudioManager::PlaySound("crawler/sound/load/turn.wav");
	int faceInt = (int)facing;
	faceInt++;
	if (faceInt == 4)
		faceInt = 0;
	facing = (FACING_INDEX)faceInt;
	state = TURNING;
	turnCurrent = 0.0f;
	oldTurn = object->localRotation.z;
	targetTurn = object->localRotation.z - 90;
	UpdatePointOfInterestTilt();

	if (!ftueHasTurned) ftueTurns++;
	dungeon->DoEventTriggerFacing(position, facing);
}

bool Crawl::DungeonPlayer::UpdateStateMoving(float delta)
{
	HandleLookTilt(delta);

	moveCurrent += delta;
	float t = MathUtils::InverseLerp(0, moveSpeed, moveCurrent);
	if (moveCurrent > moveSpeed)
	{
		//LogUtils::Log("Finished Move");
		object->SetLocalPosition(targetPosition);
		state = IDLE;
	}
	else
	{
		object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, glm::sineEaseOut(t)));
	}

	return false;
}

bool Crawl::DungeonPlayer::UpdateStateTurning(float delta)
{
	HandleLookTilt(delta);

	float t = MathUtils::InverseLerp(0, turnSpeed, turnCurrent);
	float turnPrevious = object->localRotation.z;
	if (turnCurrent > turnSpeed)
	{
		state = IDLE;
		turnDeltaPrevious = targetTurn - turnPrevious;
		object->SetLocalRotationZ(targetTurn);
		if (wasLookingAtPointOfInterest)
			SetStateRH(RHState::MoveDown);
		else
			SetStateRH(RHState::Idle);
	}
	else
	{
		float newTurnAngle = MathUtils::Lerp(oldTurn, targetTurn, MathUtils::EaseOutBounceSubtle(t));
		turnDeltaPrevious = newTurnAngle - turnPrevious;
		object->SetLocalRotationZ(newTurnAngle);
	}

	turnCurrent += delta;
	if(turnShouldApplyDeltaToFreeLook) objectView->localRotation.z -= turnDeltaPrevious;
	return false;
}

bool Crawl::DungeonPlayer::UpdateStateStairs(float delta)
{
	// If we're not facing the stairs, do that first.
	if (!facingTarget)
	{
		// We need to turn towards the enter direction
		turnCurrent += delta;
		float t = MathUtils::InverseLerp(0, turnSpeed * 2, turnCurrent);
		if (turnCurrent > turnSpeed * 2)
		{
			object->SetLocalRotationZ(targetTurn);
			facingTarget = true;
		}
		else
			object->SetLocalRotationZ(MathUtils::LerpDegrees(oldTurn, targetTurn, MathUtils::EaseOutBounceSubtle(t)));

	}
	else
	{
		stairTimeCurrent += delta;
		if (stairTimeCurrent > stairTimeTotal)
		{
			state = IDLE;
			facing = activateStairs->directionEnd;
			object->SetLocalPosition(activateStairs->endWorldPosition);
			object->SetLocalRotationZ(orientationEulers[activateStairs->directionEnd]);
			activateStairs = nullptr;
		}
		else
		{
			float t = stairTimeCurrent / stairTimeTotal;
			float moveEase = t;
			float lookEase = glm::sineEaseInOut(t);
			object->SetLocalPosition(glm::hermite(activateStairs->startWorldPosition, activateStairs->startOffset, activateStairs->endWorldPosition, activateStairs->endOffset, moveEase));
			object->SetLocalRotationZ(MathUtils::LerpDegrees(orientationEulers[activateStairs->directionStart], orientationEulers[activateStairs->directionEnd], lookEase));
		}
	}

	return false;
}

bool Crawl::DungeonPlayer::UpdateStateDying(float delta)
{
	if (fadeTimeCurrent < fadeTimeTotal)
	{
		fadeTimeCurrent += delta;
		object->AddLocalPosition({ 0,0,-delta });
		if (fadeTimeCurrent > fadeTimeTotal) fadeTimeCurrent = fadeTimeTotal;
		float t = fadeTimeCurrent / fadeTimeTotal;
		camera->postProcessFadeAmount = fadeIn ? 1 - t : t;
	}
	else
	{
		
		if (Input::Keyboard(GLFW_KEY_SPACE).Down() || Input::Keyboard(GLFW_KEY_R).Down()) // respawn
		{
			ClearFTUEPrompt(true);
			dungeon->RebuildDungeonFromSerialised(dungeon->serialised);
			Respawn();
		}
	}
	return false;
}

void Crawl::DungeonPlayer::UpdateStateTransporter(float delta)
{
	if (!facingTarget)
	{
		// We need to turn towards the enter direction
		turnCurrent += delta;
		float t = MathUtils::InverseLerp(0, turnSpeed * 2, turnCurrent);
		if (turnCurrent > turnSpeed * 2)
		{
			object->SetLocalRotationZ(targetTurn);
			facingTarget = true;
		}
		else
			object->SetLocalRotationZ(MathUtils::LerpDegrees(oldTurn, targetTurn, MathUtils::EaseOutBounceSubtle(t)));
	}

	fadeTimeCurrent += delta;
	if (fadeTimeCurrent < fadeTimeTotal)
	{
		if (fadeTimeCurrent > fadeTimeTotal) fadeTimeCurrent = fadeTimeTotal;
		float t = fadeTimeCurrent / fadeTimeTotal;

		// Walk player forward
		object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, glm::sineEaseOut(t)));

		// Fade Out
		camera->postProcessFadeAmount = t;
	}
	else if (fadeTimeCurrent < fadeTimeTotal + 0.5f)
	{
		// just hold the fade for like a little bit.
	}
	else
	{
		LoadSelectedTransporter(transporterToActivate);
	}

}

void Crawl::DungeonPlayer::SetStateRH(RHState newState)
{
	stateRH = newState;
	switch (newState)
	{
	case RHState::Idle:
	{
		animator->BlendToAnimation(animationRHIdle, 0.1f, 0.0f, true);
		break;
	}
	case RHState::DownIdle:
	{
		animator->BlendToAnimation(animationRHDownIdle, 0.1f, 0.0f, true);
		break;
	}
	case RHState::WalkForward:
	{
		animator->BlendToAnimation(animationRHWalkForward, 0.1f);
		break;
	}
	case RHState::WalkBack:
	{
		animator->BlendToAnimation(animationRHWalkBack, 0.1f);
		break;
	}
	case RHState::WalkLeft:
	{
		animator->BlendToAnimation(animationRHWalkLeft, 0.1f);
		break;
	}
	case RHState::WalkRight:
	{
		animator->BlendToAnimation(animationRHWalkRight, 0.1f);
		break;
	}
	case RHState::MoveUp:
	{
		animator->BlendToAnimation(animationRHIdle, 0.1f);
		break;
	}
	case RHState::DownWalk:
	{
		animator->BlendToAnimation(animationRHWalkForward, 0.1f, 0.0f, true);
		break;
	}
	case RHState::MoveDown:
	{
		animator->BlendToAnimation(animationRHDown, 0.1f);
		break;
	}
	case RHState::Stairs:
	{
		animator->BlendToAnimation(animationRHStairs, 0.1f);
		break;
	}
	}
}

void Crawl::DungeonPlayer::UpdateStateRH(float delta)
{
	switch (stateRH)
	{
	case RHState::Idle:
	{
		// we wall
		break;
	}
	case RHState::DownIdle:
	{
		// We ball
		break;
	}
	case RHState::WalkForward:
	case RHState::WalkBack :
	case RHState::WalkLeft:
	case RHState::WalkRight:
	{
		if (animator->IsFinished())
		{
			if (wasLookingAtPointOfInterest)
				SetStateRH(RHState::MoveDown);
			else
				SetStateRH(RHState::Idle);
		}
		break;
	}
	case RHState::MoveUp:
	{
		if (animator->IsFinished()) SetStateRH(RHState::Idle);
		break;
	}
	case RHState::DownWalk:
	{
		if (animator->IsFinished())  SetStateRH(RHState::DownIdle);
		break;
	}
	case RHState::MoveDown:
	{
		if (animator->IsFinished())  SetStateRH(RHState::DownIdle);
		break;
	}
	case RHState::Stairs:
	{
		if (animator->IsFinished())  SetStateRH(RHState::Idle);
		break;
	}
	}
}

void Crawl::DungeonPlayer::UpdatePointOfInterestTilt(bool instant)
{
	bool isPointOfInterest = dungeon->IsPlayerPointOfInterest(position + directions[facing]);
	if (wasLookingAtPointOfInterest != isPointOfInterest)
	{
		wasLookingAtPointOfInterest = isPointOfInterest;
		if (isPointOfInterest)
		{
			lookRestX = lookRestXInterest;
		}
		else
		{
			lookRestX = lookRestXDefault;
		}
		lookReturnFrom = objectView->localRotation;
		if(!instant)lookReturnTimeCurrent = 0.0f;
	}
}

void Crawl::DungeonPlayer::ContinueResettingTilt(float delta)
{
	float t = glm::clamp(lookReturnTimeCurrent / lookReturnTimeTotal, 0.0f, 1.0f);

	vec3 newRotation(0, 0, 0);
	newRotation.x = MathUtils::Lerp(lookReturnFrom.x, lookRestX, MathUtils::EaseOutBounceSubtle(t));
	newRotation.z = MathUtils::Lerp(lookReturnFrom.z, 0.0f, MathUtils::EaseOutBounceSubtle(t));
	objectView->SetLocalRotation(newRotation);

	lookReturnTimeCurrent += delta;
}

void Crawl::DungeonPlayer::SetShouldActivateTransporter(DungeonTransporter* transporter)
{
	state = TRANSPORTER;
	fadeTimeCurrent = 0.0f;
	camera->postProcessFadeColour = transporterColour;
	transporterToActivate = transporter;
	oldPosition = dungeonPosToObjectScale(position);
	oldPosition.z = playerZPosition; // correct for lobby level 2 stuff
	targetPosition = dungeonPosToObjectScale(transporter->position);
	targetPosition.z = playerZPosition; // correct for lobby level 2 stuff
}

void Crawl::DungeonPlayer::LoadSelectedTransporter(DungeonTransporter* transporter)
{
	// Mark tile as unoccupied - likely the level is going to be destroyed, but lobby 2 is persistant, and we might end up making that the case for all levels.
	DungeonTile* tile;
	if (isOnLobbyLevel2)
	{
		tile = lobbyLevel2Dungeon->GetTile(transporter->position);
		if (tile) tile->occupied = false;
	}
	// store this stuff because its about to be deleted from memory.
	string dungeonToLoad = transporter->toDungeon;
	string TransporterToGoTo = transporter->toTransporter;
	bool toLobbyLevel2 = transporter->toLobby2;

	if (!dungeon->TestDungeonExists(dungeonToLoad + dungeon->dungeonFileExtension))
	{
		LogUtils::Log("Dungeon does not exist, bailing on loading:");
		LogUtils::Log(dungeonToLoad.c_str());
		return;
	}

	// Load dungeonName - the transporter we just activated is no longer in memory.
	dungeon->Load(dungeonToLoad + ".dungeon");

	// Get Transporter By Name
	DungeonTransporter* gotoTransporter;
	if (toLobbyLevel2)
	{
		gotoTransporter = lobbyLevel2Dungeon->GetTransporter(TransporterToGoTo);
		SetLevel2(true);
	}
	else
	{
		gotoTransporter = dungeon->GetTransporter(TransporterToGoTo);
		SetLevel2(false);
	}

	// Reset previous checkpoint
	ClearCheckpoint();

	// Set player Position
	if (gotoTransporter)
	{
		SetRespawn(gotoTransporter->position, (FACING_INDEX)gotoTransporter->fromOrientation, toLobbyLevel2);
		Respawn();
	}
	else
	{
		LogUtils::Log("Unable to find transporter in new dungeon:");
		LogUtils::Log(TransporterToGoTo.c_str());
		LogUtils::Log("Spawning at default dungeon position.");
		SetRespawn(dungeon->defaultPlayerStartPosition, dungeon->defaultPlayerStartOrientation);
		Respawn();
	}

	if (TransporterToGoTo == "TutExit" && gameMenu)
	{
		gameMenu->SetLobbyReturnEnabled();
	}
}

void Crawl::DungeonPlayer::UpdatePrompts(float delta)
{
	if (promptCurrent == "" && promptNext != "")
	{
		promptCurrent = promptNext;
		promptNext = "";
		promptFadeIn = true;
		camera->promptUse = true;
	}

	camera->prompt = promptCurrent;

	if (promptFadeIn && promptNext == "")
	{
		if (promptAmount < promptFadeTime)
		{
			promptAmount += delta;
			float t = promptAmount / promptFadeTime;
			camera->promptAmount = t;
		}
		else
			camera->promptAmount = 1.0f; 
	}
	else
	{
		if (promptAmount > 0.0f)
		{
			promptAmount -= delta;
			float t = promptAmount / promptFadeTime;
			camera->promptAmount = t;
		}
		else
		{
			camera->promptAmount = 0.0f;
			if (promptNext != "")
			{
				promptCurrent = promptNext;
				promptNext = "";
				fadeIn = true;
			}
			else
			{
				promptCurrent = "";
				camera->promptUse = false;
			}
		}
	}
}

void Crawl::DungeonPlayer::UpdateFTUE()
{
	if (ftueTurns == 2 && !ftueHasTurned)
	{
		DungeonGameManager::Get()->DoFTUEEvent(DungeonGameManager::FTUEEvent::Move);
		ftueHasTurned = true;
	}
}

void Crawl::DungeonPlayer::SetFTUEPrompt(string prompt)
{
	promptNext = prompt;
	promptUse = true;
}

void Crawl::DungeonPlayer::ClearFTUEPrompt(bool instant)
{
	promptFadeIn = false;
	promptNext = "";
	if (instant)
	{
		promptCurrent = 0.0f;
		camera->promptUse = false;
	}
}

void Crawl::DungeonPlayer::DrawDebugUI()
{
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ 400, 30 });
	ImGui::Begin("Debug Information", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
	ImGui::BeginDisabled();
	ImGui::Text("Debug Build: ");
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::Text(currentDungeon->dungeonFileName.c_str());
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(50);
	ImGui::InputInt2("##Position", &position.x);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(50);
	ImGui::InputText("##Orientation", &orientationNames[facing]);
	ImGui::PopItemWidth();
	ImGui::EndDisabled();
	ImGui::End();
}

void Crawl::DungeonPlayer::SoundPlayFootstep()
{
	int stepIndex = rand() % 4;
	while (stepIndex == lastStepIndex)	stepIndex = rand() % 4;
	lastStepIndex = stepIndex;
	AudioManager::PlaySound(stepSounds[lastStepIndex]);
}

void Crawl::DungeonPlayer::Teleport(ivec2 position)
{
	state = IDLE;
	this->position = position;
	SetLevel2(respawnLevel2);
	targetPosition = { position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, playerZPosition };
	object->SetLocalPosition(targetPosition);
}

void Crawl::DungeonPlayer::Orient(FACING_INDEX facing)
{
	this->facing = facing;
	objectView->localRotation.x = lookRestX;
	object->SetLocalRotationZ(orientationEulers[facing]);
}

void Crawl::DungeonPlayer::RestartGame()
{
	DungeonTransporter* startTransporter = new DungeonTransporter();
	startTransporter->toDungeon = "crawler/dungeon/start";
	startTransporter->toTransporter = "TutExit";
	ftueInitiated = false;
	ftueEnabled = false;
	ftueHasInteracted = false;
	ftueHasLooked = false;
	ftueHasTurned = false;
	ftueTurns = 0;
	//lobbyLight = nullptr;
	//lobbyLightActivated = false;
	LoadSelectedTransporter(startTransporter);
}

void Crawl::DungeonPlayer::ReturnToLobby()
{
	DungeonTransporter* lobbyTransporter = new DungeonTransporter();
	lobbyTransporter->toDungeon = "crawler/dungeon/lobby";
	lobbyTransporter->toTransporter = "TutExit";
	LoadSelectedTransporter(lobbyTransporter);
}

void Crawl::DungeonPlayer::SetRespawn(ivec2 position, FACING_INDEX orientation, bool isLevel2)
{
	hasRespawnLocation = true;
	respawnPosition = position;
	respawnOrientation = orientation;
	respawnLevel2 = isLevel2;
}

void Crawl::DungeonPlayer::ClearRespawn()
{
	hasRespawnLocation = false;
}

void Crawl::DungeonPlayer::Respawn()
{
	if (!ftueEnabled && !ftueInitiated && dungeon->dungeonFileName == "start")
	{
		ftueEnabled = true;
		ftueInitiated = true;
		DungeonGameManager::Get()->DoFTUEEvent(DungeonGameManager::FTUEEvent::Turn);
	}

	didJustRespawn = true;
	AudioManager::PlaySound("crawler/sound/load/start.wav");
	camera->postProcessFadeAmount = 0.0f;

	if (checkpointExists)
	{
		dungeon->RebuildDungeonFromSerialised(checkpointSerialised);
		Teleport(checkpointPosition);
		Orient(checkpointFacing);
	}
	else
	{
		dungeon->ResetDungeon();
		if (hasRespawnLocation)
		{
			Teleport(respawnPosition);
			Orient(respawnOrientation);
		}
		else
		{
			Teleport(dungeon->defaultPlayerStartPosition);
			Orient(dungeon->defaultPlayerStartOrientation);
		}
	}

	hp = maxHp;
	shouldSwitchWith = nullptr;
	UpdatePointOfInterestTilt(true);
}

void Crawl::DungeonPlayer::MakeCheckpoint(FACING_INDEX direction)
{
	checkpointSerialised = dungeon->GetDungeonSerialised();
	checkpointPosition = position;
	checkpointFacing = direction;
	checkpointExists = true;
}

void Crawl::DungeonPlayer::ClearCheckpoint()
{
	checkpointSerialised.clear();
	checkpointExists = false;
}

void Crawl::DungeonPlayer::TakeDamage()
{
	hp -= 1;
}

void Crawl::DungeonPlayer::SetShouldActivateStairs(DungeonStairs* stairs)
{
	activateStairs = stairs;
	if (facing != activateStairs->directionStart)
	{
		facingTarget = false;
		turnCurrent = 0.0f;
		oldTurn = object->localRotation.z;
		targetTurn = orientationEulers[activateStairs->directionStart];
	}
	else
		facingTarget = true;
}

// Take the requested direction and offset by the direction we're facing, check for overflow, then index in to the directions array.
unsigned int Crawl::DungeonPlayer::GetMoveCardinalIndex(DIRECTION_INDEX dir)
{
	unsigned int index = (int)dir;
	index += (int)facing;
	if (index >= 4)
		index -= 4;

	return index;
}

void Crawl::DungeonPlayer::SetLevel2(bool level2)
{
	if (level2)
	{
		playerZPosition = lobbyLevel2Floor;
		currentDungeon = lobbyLevel2Dungeon;
		isOnLobbyLevel2 = true;
		respawnLevel2 = true;
	}
	else
	{
		playerZPosition = 0.0f;
		currentDungeon = dungeon;
		isOnLobbyLevel2 = false;
		respawnLevel2 = false;
	}
}

void Crawl::DungeonPlayer::DoEvent(int eventID)
{
	
}
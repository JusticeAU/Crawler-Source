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
#ifndef RELEASE
	DrawDevelopmentBuildUI();
#endif // !RELEASE

	UpdateStateRH(deltaTime);

	if (state == WAIT)
	{
		UpdateInputs();

		if (isFreeLooking) UpdateFreeLook(deltaTime);
		else ContinueResettingTilt(deltaTime);

		moveCurrent += deltaTime;
		if (moveCurrent >= moveSpeed)
		{
			state = IDLE;
			moveCurrent = 0.0f;
		}
	}
	else if (state == IDLE)
	{
		UpdateInputs();

		if (isFreeLooking) UpdateFreeLook(deltaTime);
		else ContinueResettingTilt(deltaTime);

		if (state == IDLE && UpdateStateIdle(deltaTime)) // Check state is still IDLE, freelook auto rotate might have changed it and we shouldnt consume commands if so.
			return true;
	}
	else if (state == MOVING)
	{
		UpdateInputs();

		if (isFreeLooking) UpdateFreeLook(deltaTime);
		else ContinueResettingTilt(deltaTime);

		UpdateStateMoving(deltaTime);
	}
	else if (state == TURNING)
	{
		UpdateInputs();

		if (isFreeLooking) UpdateFreeLook(deltaTime);
		else ContinueResettingTilt(deltaTime);

		UpdateStateTurning(deltaTime);
	}
	else if (state == STAIRBEARS)
	{
		UpdateInputsLooking();

		if (isFreeLooking) UpdateFreeLook(deltaTime);
		else ContinueResettingTilt(deltaTime);

		UpdateStateStairs(deltaTime);
	}
	else if (state == DYING)
	{
		ContinueResettingTilt(deltaTime);
		UpdateStateDying(deltaTime);
	}
	else if (state == TRANSPORTER)
	{
		ContinueResettingTilt(deltaTime);
		UpdateStateTransporter(deltaTime);
	}

	return false;
}

void Crawl::DungeonPlayer::UpdateInputs()
{
	UpdateInputsMovement();
	UpdateInputsLooking();
}

void Crawl::DungeonPlayer::UpdateInputsMovement()
{
	if (Input::Alias("Forward").Down()) inputBuffer = PlayerCommand::Forward;
	if (Input::Alias("Backward").Down()) inputBuffer = PlayerCommand::Back;
	if (Input::Alias("Left").Down()) inputBuffer = PlayerCommand::Left;
	if (Input::Alias("Right").Down()) inputBuffer = PlayerCommand::Right;

	if (Input::Alias("TurnLeft").Down()) inputBuffer = PlayerCommand::TurnLeft;
	if (Input::Alias("TurnRight").Down()) inputBuffer = PlayerCommand::TurnRight;

	if (Input::Alias("Interact").Down()) inputBuffer = PlayerCommand::Interact;

	if (Input::Alias("Wait").Down()) inputBuffer = PlayerCommand::Wait;

	if (Input::Alias("Reset").Down()) inputBuffer = PlayerCommand::Reset;
}

void Crawl::DungeonPlayer::UpdateInputsLooking()
{
	// The below handles swapping between gamepad and mouselook logic.
	// If input came in from the gamepad, we basicly enable 'always free look'.
	// But, if this gets enabled, then we want to check if the player started using the keyboard or mouse and disable it.
	// We only check for digital inputs (not mouse movement) because we dont want this to trigger if the player bumps the desk and gave their mouse a bit of juice.
	if (!hasLookedWithGamepad) hasLookedWithGamepad = IsLookingWithGamepad();
	else if (Input::IsAnyMouseButtonInput() || Input::GetLastInputType() == Input::InputType::Keyboard) SetFreeLookReset();

	if (Input::Alias("ResetView").Down()) SetFreeLookReset();
	if (Input::Mouse(GLFW_MOUSE_BUTTON_RIGHT).Up()) SetFreeLookReset();

	isFreeLooking = alwaysFreeLook || Input::Mouse(GLFW_MOUSE_BUTTON_RIGHT).Pressed() || hasLookedWithGamepad;
}

bool Crawl::DungeonPlayer::UpdateFreeLook(float delta, bool dontAutoReorient)
{
	// Mouse
	if (!hasLookedWithGamepad || alwaysFreeLook)
	{
		vec2 mouseDelta = -Input::GetMouseDelta() * lookSpeed;
		if (invertYAxis) mouseDelta.y = -mouseDelta.y;
		objectView->AddLocalRotation({ mouseDelta.y, 0, mouseDelta.x });
	}

	// Gamepad
	if (hasLookedWithGamepad)
	{
		// Gamepad
		bool invertLook = false;

		vec2 rightJoystick;
		rightJoystick.x = -Input::Gamepad().Axes(GLFW_GAMEPAD_AXIS_RIGHT_X);
		rightJoystick.y = -Input::Gamepad().Axes(GLFW_GAMEPAD_AXIS_RIGHT_Y);
		if (invertLook) rightJoystick.y = -rightJoystick.y;

		// deadzone
		if (abs(rightJoystick.y) < 0.2f) rightJoystick.y = 0.0;
		if (abs(rightJoystick.x) < 0.2f) rightJoystick.x = 0.0;

		if (rightJoystick.x != 0.0f || rightJoystick.y != 0.0f)
		{
			float lookSpeed = 200.0f;
			vec3 lookDelta = { rightJoystick.y * lookSpeed * delta, 0, rightJoystick.x * lookSpeed * delta };
			objectView->AddLocalRotation(lookDelta);
		}
	}

	// Clamp
	objectView->localRotation.x = glm::clamp(objectView->localRotation.x, -lookMaxX, lookMaxX);
	objectView->localRotation.z = glm::clamp(objectView->localRotation.z, -lookMaxZ, lookMaxZ);

	// Auto Re-Orient
	if (autoReOrientDuringFreeLook && state == IDLE)
	{
		if (objectView->localRotation.z > 50)
		{
			TurnLeft(true);
			return false;
		}
		else if (objectView->localRotation.z < -50)
		{
			TurnRight(true);
			return false;
		}
	}

	return false;
}

void Crawl::DungeonPlayer::SetFreeLookReset()
{
	isFreeLooking = false;
	hasLookedWithGamepad = false;

	lookReturnFrom = objectView->localRotation;
	lookReturnTimeCurrent = 0.0f;
}

void Crawl::DungeonPlayer::HandleLookTilt(float delta)
{
	if (lookReturnTimeCurrent >= lookReturnTimeTotal) return; // We're done resetting

	ContinueResettingTilt(delta);
}

bool Crawl::DungeonPlayer::IsLookingWithGamepad()
{
	if (!Input::IsGamepadConnected()) return false;

	vec2 rightJoystick;
	rightJoystick.x = -Input::Gamepad().Axes(GLFW_GAMEPAD_AXIS_RIGHT_X);
	rightJoystick.y = -Input::Gamepad().Axes(GLFW_GAMEPAD_AXIS_RIGHT_Y);

	// deadzone
	if (abs(rightJoystick.y) < 0.2f) rightJoystick.y = 0.0;
	if (abs(rightJoystick.x) < 0.2f) rightJoystick.x = 0.0;

	if (rightJoystick.x == 0.0f && rightJoystick.y == 0.0f) return false; // No Gamepad input.

	return true;
}

bool Crawl::DungeonPlayer::UpdateStateIdle(float delta)
{
	if (PostUpdateComplete)
	{
		PostUpdateComplete = false;
		dungeon->PostUpdate();
	}

	// Has point of interest changed? 
	if (rhIsDown != rhShouldBeDown)
	{
		// Should we move the hand up or down?
		if (rhShouldBeDown)
			SetStateRH(RHState::MoveDown);
		else
			SetStateRH(RHState::MoveUp);

		rhIsDown = rhShouldBeDown;
	}

	if (Input::Alias("Pause").Down() && gameMenu) // only perform this action if the gameMenu is initialised. This wont be the case in designer mode.
	{
		if (usingLevelEditor)
		{
			DungeonGameManager::Get()->ReturnToEditor();
			return false;
		}
		else
		{

			DungeonGameManager::Get()->PauseGame();
			state = MENU;
			return false;
		}
	}

	// All these checks should move to a turn processors state machine.
	if (hp <= 0)
	{
		LogUtils::Log("Died. Resetting & Respawning");
		state = DYING;
		camera->postProcessFadeColour = deathColour;
		fadeTimeCurrent = 0.0f;
		fadeIn = false;
		
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Reset);
		return false;
	}

	if (inputBuffer == PlayerCommand::Interact && currentDungeon->playerCanKickBox)
	{
		if (currentDungeon->DoKick(position, facing))
		{
			inputBuffer = PlayerCommand::None;
			ftueHasPushed = true;
			//animator->BlendToAnimation(animationNamePush, 0.1f);
			return true;
		}
	}

	if (inputBuffer == PlayerCommand::Reset)
	{
		inputBuffer == PlayerCommand::None;
		if (isOnLobbyLevel2) lobbyLevel2Dungeon->GetTile(position)->occupied = false; // because this level doesnt reset, we need to keep it tidy!!
		Respawn();
		return false;
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

	if (inputBuffer == PlayerCommand::Wait)
	{
		inputBuffer = PlayerCommand::None;
		ftueHasWaited = true;
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

	// Check for Interact
	if (inputBuffer == PlayerCommand::Interact)
	{
		inputBuffer = PlayerCommand::None;
		if (currentDungeon->DoInteractable(position, facing))
		{
			UpdatePointOfInterestTilt();
			ftueHasInteracted = true;
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
				inputBuffer = PlayerCommand::None;
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
					inputBuffer = PlayerCommand::None;
					SetFreeLookReset();
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
		inputBuffer = PlayerCommand::None;
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
	if (inputBuffer == PlayerCommand::TurnRight)
	{
		inputBuffer = PlayerCommand::None;
		TurnRight();
		return false;
	}
	if (inputBuffer == PlayerCommand::TurnLeft)
	{
		inputBuffer = PlayerCommand::None;
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
		inputBuffer == PlayerCommand::Forward ||
		inputBuffer == PlayerCommand::Back ||
		inputBuffer == PlayerCommand::Left ||
		inputBuffer == PlayerCommand::Right;
}

bool Crawl::DungeonPlayer::IsMovePressedLongEnough(float delta)
{
	if (Input::Alias("Forward").Pressed() ||
		Input::Alias("Backward").Pressed() ||
		Input::Alias("Left").Pressed() ||
		Input::Alias("Right").Pressed())
	{
		moveDelayCurrent += delta;
		if (moveDelayCurrent > moveDelay)
		{
			moveDelayCurrent = 0.0f;
			if (Input::Alias("Forward").Pressed()) inputBuffer = PlayerCommand::Forward;
			if (Input::Alias("Backward").Pressed()) inputBuffer = PlayerCommand::Back;
			if (Input::Alias("Left").Pressed()) inputBuffer = PlayerCommand::Left;
			if (Input::Alias("Right").Pressed()) inputBuffer = PlayerCommand::Right;
			return true;
		}
	}
	else moveDelayCurrent = 0.0f;

	return false;
}

int Crawl::DungeonPlayer::GetMoveDirection()
{
	if (inputBuffer == PlayerCommand::Forward) return FORWARD_INDEX;
	if (inputBuffer == PlayerCommand::Back) return BACK_INDEX;
	if (inputBuffer == PlayerCommand::Left) return LEFT_INDEX;
	if (inputBuffer == PlayerCommand::Right) return RIGHT_INDEX;
	return -1;
}

int Crawl::DungeonPlayer::GetMoveIndex()
{
	return GetMoveCardinalIndex((DIRECTION_INDEX)GetMoveDirection());
}

void Crawl::DungeonPlayer::TurnLeft(bool autoReorient)
{
	turnCurrentIsManual = !autoReorient;
	AudioManager::PlaySound("crawler/sound/load/turn.wav");
	oldTurn = orientationEulers[facing];
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

void Crawl::DungeonPlayer::TurnRight(bool autoReorient)
{
	turnCurrentIsManual = !autoReorient;
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
	float turnSpeed = turnCurrentIsManual ? turnSpeedManual : turnSpeedReOrient;
	float t = turnCurrent / turnSpeed;
	float turnPrevious = object->localRotation.z;
	if (turnCurrent > turnSpeed)
	{
		state = IDLE;
		turnDeltaPrevious = 0.0f;
		targetTurn = orientationEulers[facing];
		object->SetLocalRotationZ(targetTurn);
	}
	else
	{
		float easedT;
		if (turnCurrentIsManual) easedT = MathUtils::EaseOutBounceSubtle(t);
		else easedT = glm::sineEaseInOut(t);
		float newTurnAngle = MathUtils::Lerp(oldTurn, targetTurn, easedT);
		turnDeltaPrevious = newTurnAngle - turnPrevious;
		object->SetLocalRotationZ(newTurnAngle);
	}

	turnCurrent += delta;
	if(!turnCurrentIsManual) objectView->localRotation.z -= turnDeltaPrevious;
	return false;
}

bool Crawl::DungeonPlayer::UpdateStateStairs(float delta)
{
	// If we're not facing the stairs, do that first.
	if (!facingTarget)
	{
		// We need to turn towards the enter direction
		turnCurrent += delta;
		float t = MathUtils::InverseLerp(0, turnSpeedManual * 2, turnCurrent);
		if (turnCurrent > turnSpeedManual * 2)
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
		
		if (Input::Alias("Interact").Down() || Input::Alias("Reset").Down() || Input::Alias("Start").Down()) // respawn
		{
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
		float t = MathUtils::InverseLerp(0, turnSpeedManual * 2, turnCurrent);
		if (turnCurrent > turnSpeedManual * 2)
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
		animator->BlendToAnimation(animationRHUp, 0.1f);
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
			if (rhIsDown)
				SetStateRH(RHState::DownIdle);
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
	rhShouldBeDown = dungeon->IsPlayerPointOfInterest(position, facing);

	if (wasLookingAtPointOfInterest != rhShouldBeDown)
	{
		wasLookingAtPointOfInterest = rhShouldBeDown;
		lookRestX = rhShouldBeDown ? lookRestXInterest : lookRestXDefault;
		lookReturnFrom = objectView->localRotation;
		if(!instant) lookReturnTimeCurrent = 0.0f;
	}
}

void Crawl::DungeonPlayer::ContinueResettingTilt(float delta)
{
	if (isFreeLooking) SetFreeLookReset();

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


void Crawl::DungeonPlayer::DrawDevelopmentBuildUI()
{
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ 400, 30 });
	ImGui::Begin("Debug Information", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
	ImGui::BeginDisabled();
	ImGui::Text("Development Build: ");
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

void Crawl::DungeonPlayer::ResetPlayer()
{
	ClearCheckpoint();
	ClearRespawn();
	ftueInitiated = false;
	ftueEnabled = false;
	ftueHasInteracted = false;
	ftueHasLooked = false;
	ftueHasTurned = false;
	ftueTurns = 0;

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
	DungeonGameManager::Get()->ClearAllFTUE();

	if (!ftueEnabled && !ftueInitiated && dungeon->dungeonFileName == "start")
	{
		ftueEnabled = true;
		ftueInitiated = true;
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Turn);
		DungeonGameManager::Get()->QueueFTUEPrompt(DungeonGameFTUE::FTUEType::Move);
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
	inputBuffer = PlayerCommand::None;
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
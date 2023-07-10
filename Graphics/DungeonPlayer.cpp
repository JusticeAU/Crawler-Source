#include "DungeonPlayer.h"
#include "Input.h"
#include "Scene.h"
#include "MathUtils.h"

Crawl::DungeonPlayer::DungeonPlayer()
{
	
}

void Crawl::DungeonPlayer::Update(float deltaTime)
{
	// TODO better inject / handle this better.
	if (object == nullptr)
		object = Scene::s_instance->objects[1];

	if (state == IDLE)
	{
		glm::ivec2 coordinate = { 0, 0 };
		glm::ivec2 coordinateUnchanged = { 0, 0 }; // TO DO this sucks

		if (Input::Keyboard(GLFW_KEY_W).Pressed())
			coordinate = GetMoveCoordinate(FORWARD);
		if (Input::Keyboard(GLFW_KEY_S).Pressed())
			coordinate = GetMoveCoordinate(BACK);
		if (Input::Keyboard(GLFW_KEY_A).Pressed())
			coordinate = GetMoveCoordinate(LEFT);
		if (Input::Keyboard(GLFW_KEY_D).Pressed())
			coordinate = GetMoveCoordinate(RIGHT);

		if (coordinate != coordinateUnchanged)
		{
			if (dungeon->CanMove(position.column, position.row, coordinate.x, coordinate.y))
			{
				position.column += coordinate.x;
				position.row += coordinate.y;
				didMove = true;
			}
		}

		if (didMove)
		{
			state = MOVING;
			oldPosition = object->localPosition;
			targetPosition = { position.column * Crawl::DUNGEON_GRID_SCALE, position.row * Crawl::DUNGEON_GRID_SCALE , 0 };
			moveCurrent = 0.0f;
			didMove = false;
			return;
		}

		// Turning
		if (Input::Keyboard(GLFW_KEY_E).Pressed())
		{
			int faceInt = (int)facing;
			faceInt++;
			if (faceInt == 4)
				faceInt = 0;
			facing = (FACING)faceInt;
			state = TURNING;
			turnCurrent = 0.0f;
			oldTurn = object->localRotation.z;
			targetTurn = object->localRotation.z - 90;
		}
		if (Input::Keyboard(GLFW_KEY_Q).Pressed())
		{
			int faceInt = (int)facing;
			faceInt--;
			if (faceInt == -1)
				faceInt = 3;
			facing = (FACING)faceInt;
			state = TURNING;
			turnCurrent = 0.0f;
			oldTurn = object->localRotation.z;
			targetTurn = object->localRotation.z + 90;
		}
	}
	else if (state == MOVING)
	{
		moveCurrent += deltaTime;
		float t = MathUtils::InverseLerp(0, moveSpeed, moveCurrent);
		if (moveCurrent > moveSpeed)
		{
			object->SetLocalPosition(targetPosition);
			state = IDLE;
		}
		else
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
	}
	else if (state == TURNING)
	{
		turnCurrent += deltaTime;
		float t = MathUtils::InverseLerp(0, turnSpeed, turnCurrent);
		if (turnCurrent > turnSpeed)
		{
			object->SetLocalRotationZ(targetTurn);
			state = IDLE;
		}
		else
			object->SetLocalRotationZ(MathUtils::Lerp(oldTurn, targetTurn, t));
	}
}

// Take the requested direction and offset by the direction we're facing, check for overflow, then index in to the directions array.
glm::ivec2 Crawl::DungeonPlayer::GetMoveCoordinate(DIRECTION dir)
{
	int index = dir;
	index += facing;
	if (index >= 4)
		index -= 4;

	return directions[index];
}

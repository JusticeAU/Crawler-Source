#include "DungeonPlayer.h"
#include "Input.h"
#include "Scene.h"

Crawl::DungeonPlayer::DungeonPlayer()
{
	
}

void Crawl::DungeonPlayer::Update()
{
	// TODO better inject / handle this better.
	if (object == nullptr)
		object = Scene::s_instance->objects[1];

	glm::ivec2 coordinate = { 0, 0 };
	glm::ivec2 coordinateUnchanged = { 0, 0 }; // TO DO this sucks

	if (Input::Keyboard(GLFW_KEY_W).Down())
		coordinate = GetMoveCoordinate(FORWARD);
	if (Input::Keyboard(GLFW_KEY_S).Down())
		coordinate = GetMoveCoordinate(BACK);
	if (Input::Keyboard(GLFW_KEY_A).Down())
		coordinate = GetMoveCoordinate(LEFT);
	if (Input::Keyboard(GLFW_KEY_D).Down())
		coordinate = GetMoveCoordinate(RIGHT);


	if (Input::Keyboard(GLFW_KEY_E).Down())
	{
		int faceInt = (int)facing;
		faceInt++;
		if (faceInt == 4)
			faceInt = 0;
		facing = (FACING)faceInt;
		object->AddLocalRotation({ 0,0,-90 });
	}
	if (Input::Keyboard(GLFW_KEY_Q).Down())
	{
		int faceInt = (int)facing;
		faceInt--;
		if (faceInt == -1)
			faceInt = 3;
		facing = (FACING)faceInt;
		object->AddLocalRotation({ 0,0,90 });
	}

	if (coordinate != coordinateUnchanged)
	{
		if (dungeon->CanMove(position.column, position.row, position.column + coordinate.x, position.row + coordinate.y))
		{
			position.column += coordinate.x;
			position.row += coordinate.y;
			didMove = true;
		}
	}

	if (didMove)
	{
		object->SetLocalPosition({ position.column * Crawl::DUNGEON_GRID_SCALE, position.row * Crawl::DUNGEON_GRID_SCALE , 0 });
		didMove = false;
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

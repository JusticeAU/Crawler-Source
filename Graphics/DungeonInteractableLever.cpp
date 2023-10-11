#include "DungeonInteractableLever.h"
#include "Dungeon.h"
#include "Object.h"
#include "MathUtils.h"

Crawl::DungeonInteractableLever::~DungeonInteractableLever()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonInteractableLever::Toggle()
{
	status = !status;

	dungeon->DoActivate(activateID);
	state = State::In;
}

void Crawl::DungeonInteractableLever::SetID(unsigned int newID)
{
	id = newID;
	object->children[0]->children[0]->id = newID;
}

void Crawl::DungeonInteractableLever::UpdateTransform()
{
	object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 1.5 });
	object->SetLocalRotationZ(orientationEulers[orientation]);
}

void Crawl::DungeonInteractableLever::UpdateVisuals(float delta)
{
	float depression = 0.0f;

	if(state != State::Idle)
		buttonTime += delta;

	switch (state)
	{
	case State::Idle:
	{
		break;
	}
	case State::In:
	{
		if (buttonTime < buttonInTime)
		{
			float t = buttonTime / buttonInTime;
			depression = MathUtils::Lerp(0.0f, buttonMaxPress, t);
		}
		else
		{
			depression = buttonMaxPress;
			state = State::Hold;
			buttonTime = 0.0f;
		}
		break;
	}
	case State::Hold:
	{
		depression = buttonMaxPress;
		if (buttonTime >= buttonHoldTime)
		{
			state = State::Out;
			buttonTime = 0.0f;
		}
		break;
	}
	case State::Out:
	{
		if (buttonTime < buttonOutTime)
		{
			float t = buttonTime / buttonOutTime;
			depression = MathUtils::Lerp(buttonMaxPress, 0, t);
		}
		else
		{
			depression = 0;
			state = State::Idle;
			buttonTime = 0.0f;
		}
		break;
	}
	}

	object->children[0]->children[0]->localPosition.y = depression;
	object->children[0]->children[0]->SetDirtyTransform();
}

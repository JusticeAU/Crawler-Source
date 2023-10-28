#include "DungeonInteractableLever.h"
#include "Dungeon.h"
#include "Object.h"
#include "MathUtils.h"
#include "AudioManager.h"

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
	AudioManager::PlaySound(sfxIn, object->GetWorldSpacePosition());
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
	float depression = buttonIdlePos;

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
			depression += MathUtils::Lerp(0.0f, buttonMaxPress, t);
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
		depression += buttonMaxPress;
		if (buttonTime >= buttonHoldTime)
		{
			state = State::Out;
			buttonTime = 0.0f;
			AudioManager::PlaySound(sfxOut, object->GetWorldSpacePosition());
		}
		break;
	}
	case State::Out:
	{
		if (buttonTime < buttonOutTime)
		{
			float t = buttonTime / buttonOutTime;
			depression += MathUtils::Lerp(buttonMaxPress, 0, t);
		}
		else
		{
			depression = buttonIdlePos;
			state = State::Idle;
			buttonTime = 0.0f;
		}
		break;
	}
	}

	object->children[0]->children[0]->localPosition.y = depression;
	object->children[0]->children[0]->SetDirtyTransform();
}

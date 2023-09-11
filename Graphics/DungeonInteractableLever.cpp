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
	UpdateTransform();

	dungeon->DoActivate(activateID);
}

void Crawl::DungeonInteractableLever::SetID(unsigned int newID)
{
	id = newID;
	object->children[0]->children[0]->id = newID;
}

void Crawl::DungeonInteractableLever::UpdateTransform(bool instant)
{
	if (!instant) swingTimeCurrent = 0.0f;
	else swingTimeCurrent = swingTime;

	shouldSwing = true;
}

void Crawl::DungeonInteractableLever::UpdateVisuals(float delta)
{
	if (shouldSwing)
	{
		if (swingTimeCurrent < swingTime)
		{
			float t = swingTimeCurrent / swingTime;
			if (status)
			{
				t = glm::elasticEaseOut(t);
				float currentAngle = MathUtils::Lerp(0.0f, onRotationEuler, t);
				object->children[0]->localRotation.x = currentAngle;
			}
			else
			{
				t = glm::elasticEaseOut(t);
				float currentAngle = MathUtils::Lerp(onRotationEuler, 0.0f, t);
				object->children[0]->localRotation.x = currentAngle;
			}
			object->children[0]->SetDirtyTransform();

			swingTimeCurrent += delta;
		}
		else
		{
			float finalAngle = status ? onRotationEuler : 0.0f;
			object->children[0]->localRotation.x = finalAngle;
			object->children[0]->SetDirtyTransform();

			shouldSwing = false;
			swingTimeCurrent = 0.0f;
		}
	}
}

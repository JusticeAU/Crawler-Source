#include "DungeonDoor.h"
#include "Object.h"
#include "MathUtils.h"
#include "Window.h"
#include "AudioManager.h"
#include "DungeonHelpers.h"

#include "Scene.h"


Crawl::DungeonDoor::~DungeonDoor()
{
	if(object)
		object->markedForDeletion = true;
}

void Crawl::DungeonDoor::Toggle()
{
	open = !open;
	UpdateTransforms();
}

void Crawl::DungeonDoor::Open(bool instant)
{
	open = true;
	UpdateTransforms(instant);
}

void Crawl::DungeonDoor::Close(bool instant)
{
	open = false;
	UpdateTransforms(instant);
}

void Crawl::DungeonDoor::UpdateVisuals(float delta)
{
	if (shouldSwing)
	{
		if (swingTimeCurrent < swingTime)
		{
			float t = swingTimeCurrent / swingTime;
			if (open)
			{
				t = glm::backEaseOut(t);
				float currentAngle = MathUtils::Lerp(0.0f, openEulerAngle, t);
				object->children[0]->children[1]->SetLocalRotationZ(currentAngle);
				object->children[0]->children[2]->SetLocalRotationZ(-currentAngle);
			}
			else
			{
				t = glm::bounceEaseOut(t);
				float currentAngle = MathUtils::Lerp(openEulerAngle, 0.0f, t);
				object->children[0]->children[1]->SetLocalRotationZ(currentAngle);
				object->children[0]->children[2]->SetLocalRotationZ(-currentAngle);
			}

			swingTimeCurrent += delta;
		}
		else
		{
			float finalAngle = open ? openEulerAngle : 0.0f;
			object->children[0]->children[1]->SetLocalRotationZ(finalAngle);
			object->children[0]->children[2]->SetLocalRotationZ(-finalAngle);

			shouldSwing = false;
			swingTimeCurrent = 0.0f;
		}
	}
	if (shouldWobble)
	{
		if (wobbleTimeCurrent < wobbleTime)
		{
			wobbleTimeCurrent += delta;

			float accumulated = glfwGetTime();
			float sin = glm::sin(accumulated * 50.0f);

			float scale = 1 - (wobbleTimeCurrent / wobbleTime);
			object->children[0]->children[1]->SetLocalRotationZ(sin * 5 * scale);
			object->children[0]->children[2]->SetLocalRotationZ(-sin * 4 * scale);
		}
		else
		{
			object->children[0]->children[1]->SetLocalRotationZ(0);
			object->children[0]->children[2]->SetLocalRotationZ(0);
			shouldWobble = false;
		}

	}
}

void Crawl::DungeonDoor::UpdateTransforms(bool instant)
{
	// instant is used on level load to prime door state, for cases when a door should start open
	if (!instant) swingTimeCurrent = 0.0f;
	else swingTimeCurrent = swingTime;
	
	shouldSwing = true;
}

void Crawl::DungeonDoor::Interact()
{
	if (!isBarricaded)
	{
		AudioManager::PlaySound(wobbleSound, dungeonPosToObjectScale(object->GetWorldSpacePosition()));
		shouldWobble = true;
		wobbleTimeCurrent = 0.0f;
	}
}

void Crawl::DungeonDoor::MakeBarricaded()
{
	if (!isBarricaded)
	{
		isBarricaded = true;
		Object* barricade = Scene::CreateObject(object->children[0]);
		barricade->LoadFromJSON(ReadJSONFromDisk("crawler/model/door_barricade.object"));
		barricade->SetLocalPosition({ 0.0f, -0.05f ,0.0f });
		Close();
		UpdateTransforms(true);
	}
}

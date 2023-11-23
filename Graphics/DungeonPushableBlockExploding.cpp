#include "DungeonPushableBlockExploding.h"
#include "Scene.h"
#include "Object.h"
#include "ComponentAnimator.h"
#include "serialisation.h"


Crawl::DungeonPushableBlockExploding::DungeonPushableBlockExploding(glm::vec3 position, FACING_INDEX direction)
{
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk(modelPath + "object"));
	object->SetLocalPosition(position);
	object->SetLocalRotationZ(orientationEulersReversed[direction]);
	//object->RefreshComponents();
	ComponentAnimator* animator = (ComponentAnimator*)object->GetComponent(Component_Animator);
	animator->StartAnimation(animationName);
	animator->current->position = 23.0f;
}

Crawl::DungeonPushableBlockExploding::~DungeonPushableBlockExploding()
{
	object->markedForDeletion = true;
}
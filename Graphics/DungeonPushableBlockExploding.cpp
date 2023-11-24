#include "DungeonPushableBlockExploding.h"
#include "Scene.h"
#include "Object.h"
#include "ComponentAnimator.h"
#include "serialisation.h"


Crawl::DungeonPushableBlockExploding::DungeonPushableBlockExploding(glm::vec3 position)
{
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk(modelPathBlocker + "object"));
	object->SetLocalPosition(position);
	ComponentAnimator* animator = (ComponentAnimator*)object->GetComponent(Component_Animator);
	animator->StartAnimation(animationNameBlocker);
}

Crawl::DungeonPushableBlockExploding::DungeonPushableBlockExploding(glm::vec3 position, FACING_INDEX direction)
{
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk(modelPathMurderina + "object"));
	object->SetLocalPosition(position);
	object->SetLocalRotationZ(orientationEulersReversed[direction]);
	ComponentAnimator* animator = (ComponentAnimator*)object->GetComponent(Component_Animator);
	animator->StartAnimation(animationNameMurderina);
}

Crawl::DungeonPushableBlockExploding::~DungeonPushableBlockExploding()
{
	object->markedForDeletion = true;
}
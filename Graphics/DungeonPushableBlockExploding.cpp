#include "DungeonPushableBlockExploding.h"
#include "Scene.h"
#include "Object.h"
#include "ComponentAnimator.h"
#include "serialisation.h"


Crawl::DungeonPushableBlockExploding::DungeonPushableBlockExploding(glm::vec3 position)
{
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk(modelPath + "object"));
	object->SetLocalPosition(position);
	
	ComponentAnimator* animator = (ComponentAnimator*)object->GetComponent(Component_Animator);
	animator->StartAnimation(animationName);
}

Crawl::DungeonPushableBlockExploding::~DungeonPushableBlockExploding()
{
	object->markedForDeletion = true;
}
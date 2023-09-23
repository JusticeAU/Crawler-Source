#include "DungeonLight.h"
#include "Scene.h"
#include "DungeonHelpers.h"

Crawl::DungeonLight::~DungeonLight()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonLight::Init()
{
	object = Scene::CreateObject("Light");
	light = (ComponentLightPoint*)ComponentFactory::NewComponent(object, Component_LightPoint);
	UpdateLight();
}

void Crawl::DungeonLight::UpdateTransform()
{
	glm::vec3 worldPos = dungeonPosToObjectScale(position) + localPosition;
	object->SetLocalPosition(worldPos);
}

void Crawl::DungeonLight::UpdateLight()
{
	light->colour = colour;
	light->intensity = intensity;
}

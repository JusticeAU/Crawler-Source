#include "DungeonLight.h"
#include "Scene.h"
#include "DungeonHelpers.h"
#include "MathUtils.h"
#include "gtx/easing.hpp"

Crawl::DungeonLight::~DungeonLight()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonLight::Enable()
{
	if (isEnabled) return;

	isEnabled = true;
	object = Scene::CreateObject("Light");
	light = (ComponentLightPoint*)ComponentFactory::NewComponent(object, Component_LightPoint);
	object->components.push_back(light);
	UpdateLight();
	UpdateTransform();
}

void Crawl::DungeonLight::Disable()
{
	if (!isEnabled) return;
	isEnabled = false;
	if (object)
		object->markedForDeletion = true;
	object = nullptr;
}

void Crawl::DungeonLight::UpdateTransform()
{
	if (!object) return;

	glm::vec3 worldPos = dungeonPosToObjectScale(position) + localPosition;
	object->SetLocalPosition(worldPos);
}

void Crawl::DungeonLight::UpdateLight()
{
	light->colour = colour;
	light->intensity = intensity;
}

void Crawl::DungeonLight::Flicker()
{
	if (!isEnabled) return;

	flickerEnabled = true;
	flickerCurrent = -0.3f;
}

void Crawl::DungeonLight::ConfigureFlickerState()
{
	flickerBaseIntensity = intensity;
}

void Crawl::DungeonLight::ResetRandomFlickerTime()
{
	float percent = (float)(rand() % 100) * 0.01f;
	flickerCurrent = -(MathUtils::Lerp(flickerRepeatMin, flickerRepeatMax, percent));
}

void Crawl::DungeonLight::UpdateVisual(float delta)
{
	if (!flickerEnabled) return;

	flickerCurrent += delta;
	if (flickerCurrent > 0.0f && flickerCurrent < flickerTime)
	{
		float t = flickerCurrent / flickerTime;
		float flicker = glm::abs(glm::bounceEaseInOut(t) - 0.5f) * 2.0f;
		intensity = flickerBaseIntensity - (flickerBaseIntensity * flicker);
		UpdateLight();
	}
	else if (flickerCurrent > flickerTime)
	{
		intensity = flickerBaseIntensity;
		UpdateLight();
		
		if (flickerRepeat) ResetRandomFlickerTime();
		else (flickerEnabled) = false;
	}
}

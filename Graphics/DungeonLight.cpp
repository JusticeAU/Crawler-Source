#include "DungeonLight.h"
#include "Scene.h"
#include "DungeonHelpers.h"
#include "MathUtils.h"
#include "gtx/easing.hpp"
#include "LogUtils.h"

int Crawl::DungeonLight::lightDecorationsQuantity = 3;
string Crawl::DungeonLight::lightDecorations[] = {
	"crawler/model/decoration_light1.object",
	"crawler/model/decoration_wall_lamp.object",
	"crawler/model/decoration_standing_lamp.object"
};

glm::vec3 Crawl::DungeonLight::lightDecorationsOffsets[] =
{
	{0,0,-2.4},
	{0,0.4,0},
	{0,0.36,-1.5}
};

Crawl::DungeonLight::~DungeonLight()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonLight::Init()
{
	object = Scene::CreateObject("Light");

	ConfigureFlickerState();
	LoadDecoration();
	UpdateTransform();
	if (!startDisabled) Enable();
}

void Crawl::DungeonLight::Enable()
{
	isEnabled = true;
	if (!light)
	{
		light = (ComponentLightPoint*)ComponentFactory::NewComponent(object, Component_LightPoint);
		object->components.push_back(light);
	}
	intensityScale = 1.0f;
	UpdateLight();
	UpdateTransform();
}

void Crawl::DungeonLight::Disable()
{
	isEnabled = false;
	if (light)
	{
		light->markedForDeletion = true;
		light = nullptr;
	}

}

void Crawl::DungeonLight::LoadDecoration()
{
	// Clear off the old one
	if (lightDecoration)
		lightDecoration->markedForDeletion = true;

	// Load the new one
	if (lightDecorationID >= 0)
	{
		lightDecoration = Scene::CreateObject(object);
		lightDecoration->LoadFromJSON(ReadJSONFromDisk(lightDecorations[lightDecorationID]));
		lightDecoration->localPosition = lightDecorationsOffsets[lightDecorationID];
		object->SetLocalRotationZ(orientationEulersReversed[lightDecorationDirection]);
		lightDecorationRenderer = (ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer);
		lightDecorationRenderer->castsShadows = false;
		if (startDisabled) intensityScale = 0.0f;
	}
	else
	{
		if (lightDecoration)
			lightDecoration->markedForDeletion = true;

		lightDecoration = nullptr;
		lightDecorationRenderer = nullptr;
	}
}

void Crawl::DungeonLight::UpdateTransform()
{
	if (!object) return;

	glm::vec3 worldPos = dungeonPosToObjectScale(position) + localPosition;
	object->SetLocalPosition(worldPos);
	object->SetLocalRotationZ(orientationEulersReversed[lightDecorationDirection]);
}

void Crawl::DungeonLight::UpdateLight()
{
	if (light)
	{
		light->colour = colour;
		light->intensity = intensityCurrent;
	}

	if (lightDecorationRenderer)
	{
		lightDecorationRenderer->emissiveScale = intensityScale;
	}
}

void Crawl::DungeonLight::Flicker()
{
	if (!isEnabled) return;

	flickerEnabled = true;
	flickerCurrent = -0.3f;
}

void Crawl::DungeonLight::ConfigureFlickerState()
{
	if (flickerRepeat) flickerEnabled = true;
	intensityCurrent = intensity;
}

void Crawl::DungeonLight::ResetRandomFlickerTime()
{
	float percent = (float)(rand() % 100) * 0.01f;
	flickerCurrent = -(MathUtils::Lerp(flickerRepeatMin, flickerRepeatMax, percent));
}

void Crawl::DungeonLight::UpdateVisual(float delta)
{
	if (flickerEnabled)
	{

		flickerCurrent += delta;
		if (flickerCurrent > 0.0f && flickerCurrent < flickerTime)
		{
			float t = flickerCurrent / flickerTime;
			intensityScale = 1.0 - (glm::abs(glm::bounceEaseInOut(t) - 0.5f) * 2.0f);
			intensityCurrent = intensity - (intensity * (1.0 - intensityScale));
		}
		else if (flickerCurrent > flickerTime)
		{
			intensityCurrent = intensity;
			intensityScale = 1.0f;
		
			if (flickerRepeat) ResetRandomFlickerTime();
			else (flickerEnabled) = false;
		}
	}
	UpdateLight();
}

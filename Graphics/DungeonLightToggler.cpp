#include "DungeonLightToggler.h"
#include "Dungeon.h"
#include "DungeonLight.h"

void Crawl::DungeonLightToggler::Trigger()
{
	if (triggered) return;

	if(onlyOnce) triggered = true;

	for (auto& pointLight : dungeon->pointLights)
	{
		if (enableID != -1 && enableID == pointLight->id) pointLight->Enable();
		if (disableID != -1 && disableID == pointLight->id) pointLight->Disable();
	}
}

void Crawl::DungeonLightToggler::ReverseTrigger()
{
	if (triggered) return;

	if (onlyOnce) triggered = true;

	for (auto& pointLight : dungeon->pointLights)
	{
		if (enableID != -1 && enableID == pointLight->id) pointLight->Disable();
		if (disableID != -1 && disableID == pointLight->id) pointLight->Enable();
	}
}

#include "DungeonTransporter.h"
#include "DungeonGameManager.h"
#include "Object.h"

Crawl::DungeonTransporter::~DungeonTransporter()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonTransporter::ProcessGameManagerInteractions()
{
	if (!gameManagerInteraction) return;

	for (auto& gme : gameManagerEvents)
	{
		DungeonGameManager::Get()->RunGMEvent(gme);
	}
}

void Crawl::to_json(ordered_json& j, const DungeonTransporter& object)
{
	j = { {"name", object.name }, {"position", object.position }, {"fromOrientation", object.fromOrientation }, {"toDungeon", object.toDungeon }, {"toTransporter", object.toTransporter } };
	if (object.toLobby2) j["toLobby2"] = true;
	if (object.gameManagerInteraction)
	{
		j["gameManagerInteraction"] = true;
		for (auto& gme : object.gameManagerEvents)
		{
			j["gameManagerEvents"].push_back(gme);
		}
	}
}

void Crawl::from_json(const ordered_json& j, DungeonTransporter& object)
{
	j.at("name").get_to(object.name);
	j.at("position").get_to(object.position);
	j.at("fromOrientation").get_to(object.fromOrientation);
	j.at("toDungeon").get_to(object.toDungeon);
	j.at("toTransporter").get_to(object.toTransporter);
	if (j.contains("toLobby2")) j.at("toLobby2").get_to(object.toLobby2);
	if (j.contains("gameManagerInteraction"))
	{
		object.gameManagerInteraction = true;
		if (j.contains("gameManagerEvents"))
		{
			for (auto gme : j["gameManagerEvents"])
			{
				object.gameManagerEvents.push_back(gme);
			}
		}
	}
}
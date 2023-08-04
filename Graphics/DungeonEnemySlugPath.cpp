#include "DungeonEnemySlugPath.h"
#include "Dungeon.h"
#include "Scene.h"
#include "Object.h"

Crawl::DungeonEnemySlugPath::~DungeonEnemySlugPath()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemySlugPath::RefreshNeighbors()
{
	neighbors[NORTH_INDEX] = dungeon->GetSlugPath(position + directions[NORTH_INDEX]);
	neighbors[EAST_INDEX] = dungeon->GetSlugPath(position + directions[EAST_INDEX]);
	neighbors[SOUTH_INDEX] = dungeon->GetSlugPath(position + directions[SOUTH_INDEX]);
	neighbors[WEST_INDEX] = dungeon->GetSlugPath(position + directions[WEST_INDEX]);
}

void Crawl::DungeonEnemySlugPath::RefreshObject()
{
	unsigned int mask = 0;
	if (neighbors[NORTH_INDEX]) mask += 1;
	if (neighbors[EAST_INDEX]) mask += 4;
	if (neighbors[SOUTH_INDEX]) mask += 8;
	if (neighbors[WEST_INDEX]) mask += 2;

	if (object != nullptr)
		object->markedForDeletion = true;

	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk(DungeonEnemySlugPath::autoTileObjects[mask]));
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(DungeonEnemySlugPath::autoTileOrientations[mask]);
}


string Crawl::DungeonEnemySlugPath::autoTileObjects[16] = 
{
	"crawler/object/prototype/slug_rail_nothing.object",
	"crawler/object/prototype/slug_rail_end.object",
	"crawler/object/prototype/slug_rail_end.object",
	"crawler/object/prototype/slug_rail_corner.object",
	"crawler/object/prototype/slug_rail_end.object",
	"crawler/object/prototype/slug_rail_corner.object",
	"crawler/object/prototype/slug_rail_straight.object",
	"crawler/object/prototype/slug_rail_nothing.object",
	"crawler/object/prototype/slug_rail_end.object",
	"crawler/object/prototype/slug_rail_straight.object",
	"crawler/object/prototype/slug_rail_corner.object",
	"crawler/object/prototype/slug_rail_nothing.object",
	"crawler/object/prototype/slug_rail_corner.object",
	"crawler/object/prototype/slug_rail_nothing.object",
	"crawler/object/prototype/slug_rail_nothing.object",
	"crawler/object/prototype/slug_rail_nothing.object"
};
float Crawl::DungeonEnemySlugPath::autoTileOrientations[16] =
{
	0.0f,
	-90.0f,
	0.0f,
	-90.0f,
	-180.0f,
	180.0f,
	0.0f,
	0.0f,
	90.0f,
	90.0f,
	0.0f,
	0.0f,
	90.0f,
	0.0f,
	0.0f,
	0.0f,
};
#include "DungeonEnemySlugPath.h"
#include "Dungeon.h"
#include "Scene.h"
#include "Object.h"

Crawl::DungeonEnemySlugPath::~DungeonEnemySlugPath()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemySlugPath::AutoGenerateMask()
{
	maskTraverse = 0;
	for (int i = 0; i < 4; i++)
	{
		if (dungeon->GetSlugPath(position + directions[i])) maskTraverse += orientationMasksIndex[i];
	}
}

void Crawl::DungeonEnemySlugPath::RefreshNeighbors()
{
	for (int i = 0; i < 4; i++)
	{
		DungeonEnemySlugPath* neighbor = dungeon->GetSlugPath(position + directions[i]);
		if (neighbor)
		{
			neighbor->AutoGenerateMask();
			neighbor->RefreshObject();
		}
	}
}

void Crawl::DungeonEnemySlugPath::RefreshObject()
{
	if (object != nullptr)
		object->markedForDeletion = true;

	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk(DungeonEnemySlugPath::autoTileObjects[maskTraverse]));
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(DungeonEnemySlugPath::autoTileOrientations[maskTraverse]);
}

void Crawl::DungeonEnemySlugPath::RemoveConnectionsFromNeighbors()
{
	for (int i = 0; i < 4; i++)
	{
		DungeonEnemySlugPath* neighbor = dungeon->GetSlugPath(position + directions[i]);
		if (neighbor)
		{
			if ((neighbor->maskTraverse & orientationMasksIndex[facingIndexesReversed[i]]) == orientationMasksIndex[facingIndexesReversed[i]])
			{
				neighbor->maskTraverse -= orientationMasksIndex[facingIndexesReversed[i]];
				neighbor->RefreshObject();
			}
		}
	}
}

bool Crawl::DungeonEnemySlugPath::IsValidConfiguration()
{
	// Murderina T intersections are not valid because its ambiguous if it should go left or right.
	if (maskTraverse == 7) return false;
	if (maskTraverse == 11) return false;
	if (maskTraverse == 13) return false;
	if (maskTraverse == 14) return false;
	
	return true;
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
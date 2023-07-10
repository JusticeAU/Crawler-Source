#include "Dungeon.h"
#include "Object.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include "Scene.h"
#include "ComponentFactory.h"
#include "ModelManager.h"
#include "MaterialManager.h"
#include "serialisation.h"

Crawl::Dungeon::Dungeon()
{
	wallVariantPaths.push_back("crawler/model/tile_wall1_blockout/tile_wall1_blockout.object");
	wallVariantPaths.push_back("crawler/model/tile_wall2_blockout/tile_wall2_blockout.object");
	wallVariantPaths.push_back("crawler/model/tile_wall3_blockout/tile_wall3_blockout.object");

	InitialiseTileMap();
}

/// <summary>
/// Adds a hall to the grid.
/// </summary>
/// <param name="column"></param>
/// <param name="row"></param>
/// <returns>A reference to the newly added hall. If a hall already existing, then a nullptr is returned.</returns>
Crawl::DungeonTile* Crawl::Dungeon::AddHall(int x, int y)
{
	Column& col = halls[x];

	auto existingHall = col.row.find(y);
	if (existingHall != col.row.end())
		return nullptr; // hall existed already, no duplicating or overwriting please!

	DungeonTile newHall;
	newHall.position.x = x;
	newHall.position.y = y;
	return &col.row.emplace(y, newHall).first->second;
}

void Crawl::Dungeon::AddHall(DungeonTile& hall)
{
	Column& col = halls[hall.position.x];

	auto existingHall = col.row.find(hall.position.y);
	if (existingHall != col.row.end())
		return; // hall existed already, no duplicating or overwriting please!

	col.row.emplace(hall.position.y, hall).first->second;
}

bool Crawl::Dungeon::SetHallMask(int x, int y, int mask)
{
	DungeonTile* hall = GetHall(x, y);
	if (!hall)
		return false;

	hall->mask = mask;
	return true;
}

Crawl::DungeonTile* Crawl::Dungeon::GetHall(int x, int y)
{
	Column& col = halls[x];

	auto existingHall = col.row.find(y);
	if (existingHall == col.row.end())
		return nullptr;

	return &existingHall->second;
}

bool Crawl::Dungeon::DeleteHall(int x, int y)
{
	auto col = halls.find(x);
	if (col == halls.end())
		return false;

	auto hall = col->second.row.find(y);
	if (hall == col->second.row.end())
		return false;


	hall->second.object->markedForDeletion = true;
	col->second.row.erase(hall);
	return true;
}

void Crawl::Dungeon::CreateTileObject(DungeonTile* hall)
{
	if (hall->object != nullptr)
		hall->object->markedForDeletion = true;

	Object* obj = Scene::s_instance->DuplicateObject(tile_template, Scene::s_instance->objects[2]);
	
	// Set up wall variants
	for (int i = 0; i < 4; i++)
	{
		int variant = hall->wallVariants[i];
		if (variant > 0)
		{
			ordered_json wall = ReadJSONFromDisk(wallVariantPaths[variant-1]);
			obj->children[i + 1]->children[0]->LoadFromJSON(wall); // i+1 because this object has the floor tile in index 0;
		}
	}
	obj->SetLocalPosition({ hall->position.x * DUNGEON_GRID_SCALE, hall->position.y * DUNGEON_GRID_SCALE , 0 });

	hall->object = obj;
}

bool Crawl::Dungeon::IsOpenHall(int x, int y)
{
	auto col = halls.find(x);
	if (col == halls.end())
		return false;

	auto hall = col->second.row.find(y);
	if (hall == col->second.row.end())
		return false;

	return true;
}

bool Crawl::Dungeon::CanMove(int xFrom, int yFrom, int xDir, int yDir)
{
	bool canMove = true;
	DungeonTile* currentTile = GetHall(xFrom, yFrom);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level.

	// Check if the tile we're on allows us to move in the requested direction - Maybe I could just create some Masks for each cardinal direction and pass those around instead.
	if (yDir == 1)
		return (currentTile->mask & 1) == 1;
	if (yDir == -1)
		return (currentTile->mask & 8) == 8;
	if (xDir == 1)
		return (currentTile->mask & 4) == 4;
	if (xDir == -1)
		return (currentTile->mask & 2) == 2;

	// check destination blocked
	// TODO
	// check for edge blocked
	// TODO

	return canMove;
}

void Crawl::Dungeon::Save(std::string filename)
{
	ordered_json output;
	ordered_json halls_json;

	output["type"] = "dungeon";
	output["version"] = 1;

	for (auto& x : halls)
	{
		for (auto& y : x.second.row)
		{
			halls_json.push_back(y.second);
		}
	}
	output["halls"] = halls_json;

	WriteJSONToDisk(filename, output);
}

void Crawl::Dungeon::Load(std::string filename)
{
	auto input = ReadJSONFromDisk(filename);

	DestroySceneFromDungeonLayout();
	halls.clear();

	auto& hallsJSON = input["halls"];
	for (auto it = hallsJSON.begin(); it != hallsJSON.end(); it++)
	{
		DungeonTile hall = it.value().get<Crawl::DungeonTile>();
		AddHall(hall);
	}

	BuildSceneFromDungeonLayout();
}

void Crawl::Dungeon::InitialiseTileMap()
{
	// Load the JSON template
	ordered_json tile_layout = ReadJSONFromDisk("crawler/object/tile_layout.object");
	ordered_json tile_ground1 = ReadJSONFromDisk("crawler/model/tile_ground1_blockout/tile_ground1_blockout.object");

	tile_template = new Object(0, "Tile Template");
	tile_template->LoadFromJSON(tile_layout);
	tile_template->children[0]->children[0]->LoadFromJSON(tile_ground1);
}

void Crawl::Dungeon::DestroySceneFromDungeonLayout()
{
	for (auto& x : halls)
	{
		for (auto& y : x.second.row)
		{
			Crawl::DungeonTile* hall = &y.second;
			hall->object->markedForDeletion = true;
		}
	}
}

Object* Crawl::Dungeon::GetTileTemplate(int mask)
{
	return tile_template;
}

void Crawl::Dungeon::SetParentTileObject(Object* object)
{
	tile_template->parent = object;
}

int Crawl::Dungeon::GetAutoTileMask(int x, int y)
{
	unsigned int tile = 0;
	// test north
	if (IsOpenHall(x, y + 1))
		tile += 1;
	// test west
	if (IsOpenHall(x - 1, y))
		tile += 2;
	// test east
	if (IsOpenHall(x + 1, y))
		tile += 4;
	// test south
	if (IsOpenHall(x, y - 1))
		tile += 8;
	return tile;
}

void Crawl::Dungeon::BuildSceneFromDungeonLayout()
{
	// for each hallway, create an object.
	for (auto& column : halls)
	{
		for (auto& row : column.second.row)
		{
			Crawl::DungeonTile* hall = &row.second;
			CreateTileObject(hall);
		}
	}
}

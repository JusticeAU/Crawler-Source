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
	Object* obj = Scene::s_instance->DuplicateObject(GetTileTemplate(hall->mask), Scene::s_instance->objects[2]);
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

bool Crawl::Dungeon::CanMove(int xFrom, int yFrom, int xTo, int yTo)
{
	return IsOpenHall(xFrom, yFrom) && IsOpenHall(xTo, yTo);
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
	ordered_json tile_wall1 = ReadJSONFromDisk("crawler/model/tile_wall1_blockout/tile_wall1_blockout.object");

	tilemap[0] = new Object(0, "Enclosed");
	tilemap[0]->LoadFromJSON(tile_layout);
	tilemap[0]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[0]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[0]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[0]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[0]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	// U Bends
	tilemap[1] = new Object(0, "Open North");
	tilemap[1]->LoadFromJSON(tile_layout);
	tilemap[1]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[1]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[1]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[1]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[1]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[2] = new Object(0, "Open West");
	tilemap[2]->LoadFromJSON(tile_layout);
	tilemap[2]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[2]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[2]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[2]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[2]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[4] = new Object(0, "Open East");
	tilemap[4]->LoadFromJSON(tile_layout);
	tilemap[4]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[4]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[4]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[4]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[4]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[8] = new Object(0, "Open South");
	tilemap[8]->LoadFromJSON(tile_layout);
	tilemap[8]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[8]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[8]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[8]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[8]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	// Corners
	tilemap[3] = new Object(0, "Open North West");
	tilemap[3]->LoadFromJSON(tile_layout);
	tilemap[3]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[3]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[3]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[3]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[3]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[5] = new Object(0, "Open North East");
	tilemap[5]->LoadFromJSON(tile_layout);
	tilemap[5]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[5]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[5]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[5]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[5]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[10] = new Object(0, "Open West South");
	tilemap[10]->LoadFromJSON(tile_layout);
	tilemap[10]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[10]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[10]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[10]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[10]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[12] = new Object(0, "Open East South");
	tilemap[12]->LoadFromJSON(tile_layout);
	tilemap[12]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[12]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[12]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[12]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[12]->children[4]->children[0]->LoadFromJSON(tile_wall1);


	// tunnels
	tilemap[6] = new Object(0, "Open West East");
	tilemap[6]->LoadFromJSON(tile_layout);
	tilemap[6]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[6]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[6]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[6]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[6]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[9] = new Object(0, "Open North South");
	tilemap[9]->LoadFromJSON(tile_layout);
	tilemap[9]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[9]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[9]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[9]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[9]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	// walls
	tilemap[7] = new Object(0, "Open North West East");
	tilemap[7]->LoadFromJSON(tile_layout);
	tilemap[7]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[7]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[7]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[7]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[7]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[11] = new Object(0, "Open North West South");
	tilemap[11]->LoadFromJSON(tile_layout);
	tilemap[11]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[11]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[11]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[11]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[11]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[13] = new Object(0, "Open North East South");
	tilemap[13]->LoadFromJSON(tile_layout);
	tilemap[13]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[13]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[13]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[13]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	tilemap[13]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[14] = new Object(0, "Open West East South");
	tilemap[14]->LoadFromJSON(tile_layout);
	tilemap[14]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	tilemap[14]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[14]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[14]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[14]->children[4]->children[0]->LoadFromJSON(tile_wall1);

	tilemap[15] = new Object(0, "Open");
	tilemap[15]->LoadFromJSON(tile_layout);
	tilemap[15]->children[0]->children[0]->LoadFromJSON(tile_ground1);
	//tilemap[15]->children[1]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[15]->children[2]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[15]->children[3]->children[0]->LoadFromJSON(tile_wall1);
	//tilemap[15]->children[4]->children[0]->LoadFromJSON(tile_wall1);
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
	return tilemap[mask];
}

void Crawl::Dungeon::SetParentTileObject(Object* object)
{
	for (int i = 0; i < 16; i++)
		tilemap[i]->parent = object;
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

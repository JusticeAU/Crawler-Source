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
	Scene::s_instance->dungeon->SetParentTileObject(Scene::s_instance->objects[2]);
	Object* obj = Scene::s_instance->DuplicateObject(GetTileTemplate(hall->mask));
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
	ComponentModel* model;
	ComponentRenderer* renderer;

	tilemap[0] = new Object(0, "Enclosed");
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[0], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallClosed.fbx");
	model->modelName = "models/crawl/blockout/hallClosed.fbx";
	tilemap[0]->components.push_back(model);
	renderer = (ComponentRenderer*)ComponentFactory::NewComponent(tilemap[0], Component_Renderer);
	renderer->materialArray.resize(5);
	for(int i = 0; i < 5; i++)
	{
		renderer->materialArray[i] = MaterialManager::GetMaterial("models/crawl/blockout/hall.material");
	}
	tilemap[0]->components.push_back(renderer);

	// U Bends
	tilemap[1] = new Object(0, "Open North");

	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[1], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallU.fbx");
	model->modelName = "models/crawl/blockout/hallU.fbx";
	tilemap[1]->components.push_back(model);
	tilemap[1]->components.push_back(renderer);
	tilemap[2] = new Object(0, "Open West");
	tilemap[2]->SetLocalRotation({ 0,0,90 });
	tilemap[2]->components.push_back(model);
	tilemap[2]->components.push_back(renderer);
	tilemap[4] = new Object(0, "Open East");
	tilemap[4]->SetLocalRotation({ 0,0,-90 });
	tilemap[4]->components.push_back(model);
	tilemap[4]->components.push_back(renderer);
	tilemap[8] = new Object(0, "Open South");
	tilemap[8]->SetLocalRotation({ 0,0,180 });
	tilemap[8]->localRotation.z = 180.0f;
	tilemap[8]->components.push_back(model);
	tilemap[8]->components.push_back(renderer);

	// Corners
	tilemap[3] = new Object(0, "Open North West");
	tilemap[3]->SetLocalRotation({ 0,0,-90 });
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[3], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallCorner.fbx");
	model->modelName = "models/crawl/blockout/hallhallCorner.fbx";
	tilemap[3]->components.push_back(model);
	tilemap[3]->components.push_back(renderer);
	tilemap[5] = new Object(0, "Open North East");
	tilemap[5]->SetLocalRotation({ 0,0,180 });
	tilemap[5]->components.push_back(model);
	tilemap[5]->components.push_back(renderer);
	tilemap[10] = new Object(0, "Open West South");
	tilemap[10]->components.push_back(model);
	tilemap[10]->components.push_back(renderer);
	tilemap[12] = new Object(0, "Open East South");
	tilemap[12]->SetLocalRotation({ 0,0,90 });
	tilemap[12]->components.push_back(model);
	tilemap[12]->components.push_back(renderer);


	// tunnels
	tilemap[6] = new Object(0, "Open West East");
	tilemap[6]->SetLocalRotation({ 0,0,90 });
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[3], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallSides.fbx");
	model->modelName = "models/crawl/blockout/hallSides.fbx";
	tilemap[6]->components.push_back(model);
	tilemap[6]->components.push_back(renderer);
	tilemap[9] = new Object(0, "Open North South");
	tilemap[9]->components.push_back(model);
	tilemap[9]->components.push_back(renderer);

	// walls
	tilemap[7] = new Object(0, "Open North West East");
	tilemap[7]->SetLocalRotation({ 0,0,90 });
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[7], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallWall.fbx");
	model->modelName = "models/crawl/blockout/hallWall.fbx";
	tilemap[7]->components.push_back(model);
	tilemap[7]->components.push_back(renderer);
	tilemap[11] = new Object(0, "Open North West South");
	tilemap[11]->SetLocalRotation({ 0,0,180 });
	tilemap[11]->components.push_back(model);
	tilemap[11]->components.push_back(renderer);
	tilemap[13] = new Object(0, "Open North East South");
	tilemap[13]->components.push_back(model);
	tilemap[13]->components.push_back(renderer);
	tilemap[14] = new Object(0, "Open West East South");
	tilemap[14]->SetLocalRotation({ 0,0,-90 });
	tilemap[14]->components.push_back(model);
	tilemap[14]->components.push_back(renderer);

	tilemap[15] = new Object(0, "Open");
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[15], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallOpen.fbx");
	model->modelName = "models/crawl/blockout/hallOpen.fbx";
	tilemap[15]->components.push_back(model);
	tilemap[15]->components.push_back(renderer);

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

#include "Dungeon.h"
#include "DungeonHelpers.h"
#include "DungeonInteractableLever.h"
#include "DungeonDoor.h"
#include "DungeonPlayer.h"
#include "Object.h"
#include "DungeonActivatorPlate.h"
#include "DungeonTransporter.h"

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
/// Adds a dungeonTile to the grid.
/// </summary>
/// <param name="column"></param>
/// <param name="row"></param>
/// <returns>A reference to the newly added dungeonTile. If a dungeonTile already existing, then a nullptr is returned.</returns>
Crawl::DungeonTile* Crawl::Dungeon::AddTile(ivec2 position)
{
	Column& col = tiles[position.x];

	auto existingTile = col.row.find(position.y);
	if (existingTile != col.row.end())
		return nullptr; // dungeonTile existed already, no duplicating or overwriting please!

	DungeonTile newTile;
	newTile.position = position;
	return &col.row.emplace(position.y, newTile).first->second;
}

void Crawl::Dungeon::AddTile(DungeonTile& dungeonTile)
{
	Column& col = tiles[dungeonTile.position.x];

	auto existingTile = col.row.find(dungeonTile.position.y);
	if (existingTile != col.row.end())
		return; // dungeonTile existed already, no duplicating or overwriting please!

	col.row.emplace(dungeonTile.position.y, dungeonTile).first->second;
}

bool Crawl::Dungeon::SetTileMask(ivec2 position, int mask)
{
	DungeonTile* tile = GetTile(position);
	if (!tile)
		return false;

	tile->mask = mask;
	return true;
}

Crawl::DungeonTile* Crawl::Dungeon::GetTile(ivec2 pos)
{
	Column& col = tiles[pos.x];

	auto existingTile = col.row.find(pos.y);
	if (existingTile == col.row.end())
		return nullptr;

	return &existingTile->second;
}

bool Crawl::Dungeon::DeleteTile(ivec2 position)
{
	auto col = tiles.find(position.x);
	if (col == tiles.end())
		return false;

	auto tile = col->second.row.find(position.y);
	if (tile == col->second.row.end())
		return false;


	tile->second.object->markedForDeletion = true;
	col->second.row.erase(tile);
	return true;
}

void Crawl::Dungeon::CreateTileObject(DungeonTile* tile)
{
	if (tile->object != nullptr)
		tile->object->markedForDeletion = true;

	Object* obj = Scene::s_instance->DuplicateObject(tile_template, Scene::s_instance->objects[2]);
	
	// Set up wall variants
	for (int i = 0; i < 4; i++)
	{
		int variant = tile->wallVariants[i];
		if (variant > 0)
		{
			ordered_json wall = ReadJSONFromDisk(wallVariantPaths[variant-1]);
			obj->children[i + 1]->children[0]->LoadFromJSON(wall); // i+1 because this object has the floor tile in index 0;
		}
	}
	obj->SetLocalPosition({ tile->position.x * DUNGEON_GRID_SCALE, tile->position.y * DUNGEON_GRID_SCALE , 0 });

	tile->object = obj;
}

bool Crawl::Dungeon::IsOpenTile(ivec2 position)
{
	auto col = tiles.find(position.x);
	if (col == tiles.end())
		return false;

	auto tile = col->second.row.find(position.y);
	if (tile == col->second.row.end())
		return false;

	return true;
}

bool Crawl::Dungeon::CanMove(glm::ivec2 fromPos, int directionIndex)
{
	glm::ivec2 toPos = fromPos + directions[directionIndex];
	
	unsigned int directionMask;
	if (directionIndex == NORTH_INDEX) directionMask = NORTH_MASK;
	else if (directionIndex == EAST_INDEX) directionMask = EAST_MASK;
	else if (directionIndex == SOUTH_INDEX) directionMask = SOUTH_MASK;
	else directionMask = WEST_MASK;
	unsigned int reverseDirectionIndex = directionIndex + 2;
	if (reverseDirectionIndex > 3) reverseDirectionIndex -= 4;

	bool canMove = true;
	DungeonTile* currentTile = GetTile(fromPos);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level!

	// Check if the tile we're on allows us to move in the requested direction - Maybe I could just create some Masks for each cardinal direction and pass those around instead.
	canMove = (currentTile->mask & directionMask) == directionMask;
	
	if (!canMove)
		return canMove;

	// check for edge blocked - Doors!
	// Check tile we're on
	DungeonDoor* doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == fromPos && activatable[i]->orientation == directionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open)
		return false;

	// check tile we want to move to.
	doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == toPos && activatable[i]->orientation == reverseDirectionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open)
		return false;

	// check destination blocked
	// TODO

	return canMove;
}

void Crawl::Dungeon::DoInteractable(unsigned int id)
{
	for(int i = 0; i < interactables.size(); i++)
	{
		if (id == interactables[i]->id)
		{
			interactables[i]->Toggle();
			break;
		}
	}
}

Crawl::DungeonInteractableLever* Crawl::Dungeon::CreateLever(ivec2 position, unsigned int directionIndex, unsigned int id, unsigned int doorID, bool startStatus)
{
	DungeonInteractableLever* lever = new DungeonInteractableLever();
	lever->position = position;
	lever->orientation = directionIndex;
	lever->dungeon = this;
	lever->startStatus = startStatus;

	// Load lever Objects from JSON
	ordered_json lever_objectJSON = ReadJSONFromDisk("crawler/object/interactable_lever.object");
	ordered_json lever_modelJSON = ReadJSONFromDisk("crawler/model/interactable_lever_blockout/interactable_lever_blockout.object");

	// load Model object in to Model child object
	Object* lever_object = Scene::CreateObject();
	lever_object->LoadFromJSON(lever_objectJSON);
	lever->object = lever_object;
	lever_object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 2.5 });
	Object* lever_model = Scene::CreateObject(lever_object->children[0]);
	lever_model->LoadFromJSON(lever_modelJSON);
	if (directionIndex == EAST_INDEX) lever_object->localRotation.z = -90.0f;
	else if (directionIndex == WEST_INDEX) lever_object->localRotation.z = 90.0f;
	else if (directionIndex == SOUTH_INDEX) lever_object->localRotation.z = 180.0f;
	lever_object->children[0]->dirtyTransform = true;
	lever->SetID(id);
	lever->activateID = doorID;
	interactables.push_back(lever);
	return lever;
}

void Crawl::Dungeon::DoActivate(unsigned int id)
{
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->id == id)
			activatable[i]->Toggle();
	}
}

void Crawl::Dungeon::DoActivate(unsigned int id, bool on)
{
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->id == id)
			activatable[i]->Toggle(on);
	}
}

Crawl::DungeonDoor* Crawl::Dungeon::CreateDoor(ivec2 position, unsigned int directionIndex, unsigned int id, bool startOpen)
{
	DungeonDoor* door = new DungeonDoor();
	door->position = position;
	door->orientation = directionIndex;
	door->id = id;
	door->startOpen = startOpen;
	door->open = startOpen;

	ordered_json door_objectJSON = ReadJSONFromDisk("crawler/object/interactable_door.object");
	Object* door_object = Scene::CreateObject();
	door_object->LoadFromJSON(door_objectJSON);
	door->object = door_object;
	door_object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	ordered_json door_modelJSON = ReadJSONFromDisk("crawler/model/door_jail_blockout/door_jail_blockout.object");
	Object* door_model = Scene::CreateObject(door_object->children[0]);
	door_model->LoadFromJSON(door_modelJSON);
	if (directionIndex == EAST_INDEX) door_object->localRotation.z = -90.0f;
	else if (directionIndex == EAST_INDEX) door_object->localRotation.z = 90.0f;
	else if (directionIndex == EAST_INDEX) door_object->localRotation.z = 180.0f;
	door_object->children[0]->dirtyTransform = true;
	activatable.push_back(door);
	return door;
}

Crawl::DungeonActivatorPlate* Crawl::Dungeon::CreatePlate(ivec2 position, unsigned int activateID)
{
	DungeonActivatorPlate* plate = new DungeonActivatorPlate();
	plate->position = position;
	plate->activateID = activateID;
	plate->dungeon = this;
	plate->object = Scene::CreateObject();
	plate->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/interactable_floortile.object"));
	plate->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_floorplate/interactable_floorplate.object"));
	plate->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	activatorPlates.push_back(plate);
	return plate;
}

Crawl::DungeonTransporter* Crawl::Dungeon::CreateTransporter(ivec2 position)
{
	DungeonTransporter* transporter = new DungeonTransporter();
	transporter->position = position;
	transporter->object = Scene::CreateObject();
	transporter->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/exit.object"));
	transporter->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 1 });
	transporterPlates.push_back(transporter);
	return transporter;
}

Crawl::DungeonTransporter* Crawl::Dungeon::GetTransporter(string transporterName)
{
	for (auto& transporter : transporterPlates)
		if (transporter->name == transporterName) return transporter;

	return nullptr;
}

void Crawl::Dungeon::Save(std::string filename)
{
	BuildSerialised();
	WriteJSONToDisk(filename, serialised);
}

void Crawl::Dungeon::Load(std::string filename)
{
	serialised = ReadJSONFromDisk(filename);
	RebuildFromSerialised();
}

void Crawl::Dungeon::BuildSerialised()
{
	serialised.clear();

	ordered_json tiles_json;

	serialised["type"] = "dungeon";
	serialised["version"] = 1;
	serialised["defaultPosition"] = defaultPlayerStartPosition;
	serialised["defaultOrientation"] = defaultPlayerStartOrientation;

	for (auto& x : tiles)
	{
		for (auto& y : x.second.row)
			tiles_json.push_back(y.second);
	}
	serialised["tiles"] = tiles_json;

	ordered_json levers_json;
	for (auto& interactable : interactables)
		levers_json.push_back(*interactable);
	serialised["levers"] = levers_json;

	ordered_json doors_json;
	for (auto& door : activatable)
		doors_json.push_back(*door);
	serialised["doors"] = doors_json;

	ordered_json plates_json;
	for (auto& plate : activatorPlates)
		plates_json.push_back(*plate);
	serialised["plates"] = plates_json;

	ordered_json transporters_json;
	for (auto& plate : transporterPlates)
		transporters_json.push_back(*plate);
	serialised["transporters"] = transporters_json;
}

void Crawl::Dungeon::RebuildFromSerialised()
{
	DestroySceneFromDungeonLayout();
	tiles.clear();

	if (serialised.contains("defaultPosition"))
		serialised.at("defaultPosition").get_to(defaultPlayerStartPosition);
	else
		defaultPlayerStartPosition = { 0,0 };

	if (serialised.contains("defaultOrientation"))
		serialised.at("defaultOrientation").get_to(defaultPlayerStartOrientation);
	else
		defaultPlayerStartOrientation = EAST_INDEX;


	auto& tiles_json = serialised["tiles"];
	for (auto it = tiles_json.begin(); it != tiles_json.end(); it++)
	{
		DungeonTile tile = it.value().get<Crawl::DungeonTile>();
		AddTile(tile);
	}

	auto& levers_json = serialised["levers"];
	for (auto it = levers_json.begin(); it != levers_json.end(); it++)
	{
		DungeonInteractableLever lever = it.value().get<Crawl::DungeonInteractableLever>();
		CreateLever(lever.position, lever.orientation, lever.id, lever.activateID, lever.startStatus);
	}

	auto& doors_json = serialised["doors"];
	for (auto it = doors_json.begin(); it != doors_json.end(); it++)
	{
		DungeonDoor door = it.value().get<Crawl::DungeonDoor>();
		CreateDoor(door.position, door.orientation, door.id, door.startOpen);
	}

	auto& plates_json = serialised["plates"];
	for (auto it = plates_json.begin(); it != plates_json.end(); it++)
	{
		DungeonActivatorPlate plate = it.value().get<Crawl::DungeonActivatorPlate>();
		CreatePlate(plate.position, plate.activateID);
	}

	auto& transporters_json = serialised["transporters"];
	for (auto it = transporters_json.begin(); it != transporters_json.end(); it++)
	{
		DungeonTransporter transporter = it.value().get<Crawl::DungeonTransporter>();
		DungeonTransporter* newTransporter = CreateTransporter(transporter.position);
		newTransporter->name = transporter.name;
		newTransporter->toDungeon = transporter.toDungeon;
		newTransporter->toTransporter = transporter.toTransporter;
		newTransporter->fromOrientation = transporter.fromOrientation;
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
	for (auto& x : tiles)
	{
		for (auto& y : x.second.row)
		{
			Crawl::DungeonTile* tile = &y.second;
			tile->object->markedForDeletion = true;
		}
	}

	for (int i = 0; i < interactables.size(); i++)
		delete interactables[i];
	interactables.clear();

	for (int i = 0; i < activatable.size(); i++)
		delete activatable[i];
	activatable.clear();

	for (int i = 0; i < activatorPlates.size(); i++)
		delete activatorPlates[i];
	activatorPlates.clear();

	for (int i = 0; i < transporterPlates.size(); i++)
		delete transporterPlates[i];
	transporterPlates.clear();

}

Object* Crawl::Dungeon::GetTileTemplate(int mask)
{
	return tile_template;
}

void Crawl::Dungeon::SetParentTileObject(Object* object)
{
	tile_template->parent = object;
}

unsigned int Crawl::Dungeon::GetReverseDirectionMask(unsigned int direction)
{
	switch (direction)
	{
	case NORTH_MASK:
		return SOUTH_MASK;
	case SOUTH_MASK:
		return NORTH_MASK;
	case EAST_MASK:
		return WEST_MASK;
	case WEST_MASK:
		return EAST_MASK;
	default:
		return 0; // Shouldn't get here.
	}
}

void Crawl::Dungeon::Update()
{
	// test all activator plates
	for (auto& tileTest : activatorPlates)
		tileTest->TestPosition();


	// update all doors
	for (auto& door : activatable)
		door->Update();

	// test all transporters
	DungeonTransporter* activateTransporter = nullptr;
	for (auto& transporter : transporterPlates)
	{
		if (transporter->position == player->GetPosition())
		{
			activateTransporter = transporter;
			break;
		}
	}
	if (activateTransporter)
	{
		string dungeonToLoad = activateTransporter->toDungeon;
		string TransporterToGoTo = activateTransporter->toTransporter;
		// Load dungeonName
		Load(dungeonToLoad + ".dungeon");

		// Get Transporter By Name
		DungeonTransporter* gotoTransporter = GetTransporter(TransporterToGoTo);

		// Set player Position
		if (gotoTransporter)
		{
			player->SetRespawn(gotoTransporter->position, (FACING_INDEX)gotoTransporter->fromOrientation);
			player->Respawn();
		}
		else
			LogUtils::Log("Unable to find transporter in new dungeon");
	}

}

unsigned int Crawl::Dungeon::GetAutoTileMask(ivec2 position)
{
	unsigned int tile = 0;
	// test north
	if (IsOpenTile(position + NORTH_COORDINATE))
		tile += NORTH_MASK;
	// test west
	if (IsOpenTile(position + WEST_COORDINATE))
		tile += WEST_MASK;
	// test east
	if (IsOpenTile(position + EAST_COORDINATE))
		tile += EAST_MASK;
	// test south
	if (IsOpenTile(position + SOUTH_COORDINATE))
		tile += SOUTH_MASK;
	return tile;
}

void Crawl::Dungeon::BuildSceneFromDungeonLayout()
{
	// for each tile, create an object.
	for (auto& column : tiles)
	{
		for (auto& row : column.second.row)
		{
			Crawl::DungeonTile* tile = &row.second;
			CreateTileObject(tile);
		}
	}
}

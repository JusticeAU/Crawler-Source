#include "Dungeon.h"
#include "Object.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include "Scene.h"
#include "ComponentFactory.h"
#include "ModelManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"

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
Crawl::Hall* Crawl::Dungeon::AddHall(int column, int row)
{
	Column& col = halls[column];

	auto existingHall = col.row.find(row);
	if (existingHall != col.row.end())
		return nullptr; // hall existed already, no duplicating or overwriting please!

	Hall newHall;
	newHall.column = column;
	newHall.row = row;
	return &col.row.emplace(row, newHall).first->second;
}

Crawl::Hall* Crawl::Dungeon::GetHall(int column, int row)
{
	Column& col = halls[column];

	auto existingHall = col.row.find(row);
	if (existingHall == col.row.end())
		return nullptr;

	return &existingHall->second;
}

bool Crawl::Dungeon::DeleteHall(int column, int row)
{
	auto col = halls.find(column);
	if (col == halls.end())
		return false;

	auto hall = col->second.row.find(row);
	if (hall == col->second.row.end())
		return false;


	hall->second.object->markedForDeletion = true;
	col->second.row.erase(hall);
	return true;
}

void Crawl::Dungeon::CreateTileObject(Hall* hall)
{
	unsigned int mask = GetTileMask(hall->column, hall->row);

	Object* obj = Scene::s_instance->DuplicateObject(GetTileTemplate(mask));
	obj->localTransform[3][0] = hall->column * DUNGEON_GRID_SCALE;
	obj->localTransform[3][2] = hall->row * DUNGEON_GRID_SCALE;
	obj->dirtyTransform = true;

	hall->object = obj;
}

bool Crawl::Dungeon::IsOpenHall(int column, int row)
{
	auto col = halls.find(column);
	if (col == halls.end())
		return false;

	auto hall = col->second.row.find(row);
	if (hall == col->second.row.end())
		return false;

	return true;
}

void Crawl::Dungeon::Save(std::string filename)
{
	std::ofstream outFile(filename, std::ios_base::trunc);

	if (outFile.is_open())
	{
		// Save version
		FileUtils::StrWriteLine(outFile, "version");
		FileUtils::StrWriteInt(outFile, 1);

		// save hall coordinates
		for (auto& column : halls)
		{
			for (auto& row : column.second.row)
			{
				FileUtils::StrWriteLine(outFile, "hall");
				FileUtils::StrWriteInt(outFile, row.second.column);
				FileUtils::StrWriteInt(outFile, row.second.row);
			}
		}
	}

}

void Crawl::Dungeon::Load(std::string filename)
{
	std::ifstream inFile(filename);
	std::string line;
	if (inFile.is_open())
	{
		halls.clear();

		int version = 0;
		while (std::getline(inFile, line))
		{
			if (line == "version")
			{
				std::getline(inFile, line);
				version = atoi(line.c_str());
			}
			else if (line == "hall")
			{
				int column, row;

				std::getline(inFile, line);
				column = atoi(line.c_str());

				std::getline(inFile, line);
				row = atoi(line.c_str());
				AddHall(column, row);
			}
		}

		BuildSceneFromDungeonLayout();
	}
	else
		LogUtils::Log("Failed to load");
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
	renderer->shader = ShaderManager::GetShaderProgram("shaders/lambertLitSingleLight");
	renderer->texture = TextureManager::GetTexture("models/crawl/blockout/Mage_Room.png");
	renderer->textureName = "models/crawl/blockout/Mage_Room.png";
	tilemap[0]->components.push_back(renderer);

	// U Bends
	tilemap[1] = new Object(0, "Open North");
	tilemap[1]->eulerRotation.y = -90.0f;
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[1], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallU.fbx");
	model->modelName = "models/crawl/blockout/hallU.fbx";
	tilemap[1]->components.push_back(model);
	tilemap[1]->components.push_back(renderer);
	tilemap[2] = new Object(0, "Open West");
	tilemap[2]->eulerRotation.y = 180.0f;
	tilemap[2]->components.push_back(model);
	tilemap[2]->components.push_back(renderer);
	tilemap[4] = new Object(0, "Open East");
	tilemap[4]->components.push_back(model);
	tilemap[4]->components.push_back(renderer);
	tilemap[8] = new Object(0, "Open South");
	tilemap[8]->eulerRotation.y = 90.0f;
	tilemap[8]->components.push_back(model);
	tilemap[8]->components.push_back(renderer);

	// Corners
	tilemap[3] = new Object(0, "Open North West");
	tilemap[3]->eulerRotation.y = -90.0f;
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[3], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallCorner.fbx");
	model->modelName = "models/crawl/blockout/hallhallCorner.fbx";
	tilemap[3]->components.push_back(model);
	tilemap[3]->components.push_back(renderer);
	tilemap[5] = new Object(0, "Open North East");
	tilemap[5]->components.push_back(model);
	tilemap[5]->components.push_back(renderer);
	tilemap[10] = new Object(0, "Open West South");
	tilemap[10]->eulerRotation.y = 180.0f;
	tilemap[10]->components.push_back(model);
	tilemap[10]->components.push_back(renderer);
	tilemap[12] = new Object(0, "Open East South");
	tilemap[12]->eulerRotation.y = 90.0f;
	tilemap[12]->components.push_back(model);
	tilemap[12]->components.push_back(renderer);


	// tunnels
	tilemap[6] = new Object(0, "Open West East");
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[3], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallSides.fbx");
	model->modelName = "models/crawl/blockout/hallSides.fbx";
	tilemap[6]->components.push_back(model);
	tilemap[6]->components.push_back(renderer);
	tilemap[9] = new Object(0, "Open North South");
	tilemap[9]->eulerRotation.y = 90.0f;
	tilemap[9]->components.push_back(model);
	tilemap[9]->components.push_back(renderer);

	// walls
	tilemap[7] = new Object(0, "Open North West East");
	tilemap[7]->eulerRotation.y = -90.0f;
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[7], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallWall.fbx");
	model->modelName = "models/crawl/blockout/hallWall.fbx";
	tilemap[7]->components.push_back(model);
	tilemap[7]->components.push_back(renderer);
	tilemap[11] = new Object(0, "Open North West South");
	tilemap[11]->eulerRotation.y = 180.0f;
	tilemap[11]->components.push_back(model);
	tilemap[11]->components.push_back(renderer);
	tilemap[13] = new Object(0, "Open North East South");
	tilemap[13]->components.push_back(model);
	tilemap[13]->components.push_back(renderer);
	tilemap[14] = new Object(0, "Open West East South");
	tilemap[14]->eulerRotation.y = 90.0f;
	tilemap[14]->components.push_back(model);
	tilemap[14]->components.push_back(renderer);

	tilemap[15] = new Object(0, "Open");
	model = (ComponentModel*)ComponentFactory::NewComponent(tilemap[15], Component_Model);
	model->model = ModelManager::GetModel("models/crawl/blockout/hallOpen.fbx");
	model->modelName = "models/crawl/blockout/hallOpen.fbx";
	tilemap[15]->components.push_back(model);
	renderer = (ComponentRenderer*)ComponentFactory::NewComponent(tilemap[15], Component_Renderer);
	renderer->shader = ShaderManager::GetShaderProgram("shaders/lambertLitSingleLight");
	renderer->texture = TextureManager::GetTexture("models/crawl/blockout/Mage_Room.png");
	renderer->textureName = "models/crawl/blockout/Mage_Room.png";
	tilemap[15]->components.push_back(renderer);

}

Object* Crawl::Dungeon::GetTileTemplate(int mask)
{
	return tilemap[mask];
}

int Crawl::Dungeon::GetTileMask(int col, int row)
{
	unsigned int tile = 0;
	// test north
	if (IsOpenHall(col, row + 1))
	{
		tile += 1;
		LogUtils::Log("There is a tile 'north' of us");
	}
	// test west
	if (IsOpenHall(col - 1, row))
	{
		tile += 2;
		LogUtils::Log("There is a tile 'west' of us");
	}
	// test east
	if (IsOpenHall(col + 1, row))
	{
		tile += 4;
		LogUtils::Log("There is a tile 'east' of us");
	}
	// test south
	if (IsOpenHall(col, row - 1))
	{
		tile += 8;
		LogUtils::Log("There is a tile 'south' of us");
	}
	return tile;
}

void Crawl::Dungeon::BuildSceneFromDungeonLayout()
{
	// delete all objects representing dungeon bits
	while (Scene::s_instance->objects.size() > 1) // this is shit
		Scene::s_instance->objects.erase(Scene::s_instance->objects.end() - 1);

	// for each hallway, create an object.
	for (auto& column : halls)
	{
		for (auto& row : column.second.row)
		{
			Crawl::Hall* hall = &row.second;
			CreateTileObject(hall);
		}
	}
}

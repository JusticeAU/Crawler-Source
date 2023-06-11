#include "Dungeon.h"
#include "FileUtils.h"
#include "Object.h"

/// <summary>
/// Adds a hall to the grid.
/// </summary>
/// <param name="column"></param>
/// <param name="row"></param>
/// <returns>a reference to the newly added hall. If a hall already existing, then a nullptr is returned.</returns>
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

void Crawl::Dungeon::DeleteHall(int column, int row)
{
	auto col = halls.find(column);
	if (col == halls.end())
		return;

	auto hall = col->second.row.find(row);
	if (hall == col->second.row.end())
		return;


	hall->second.object->markedForDeletion = true;
	col->second.row.erase(hall);
}

void Crawl::Dungeon::Save()
{
	std::ofstream outFile("crawl/dungeons/test.dungeon",std::ios_base::trunc);

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

void Crawl::Dungeon::Load()
{
	halls.clear();

	int version = 0;

	std::ifstream inFile("crawl/dungeons/test.dungeon");
	std::string line;
	if (inFile.is_open())
	{
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
	}
}

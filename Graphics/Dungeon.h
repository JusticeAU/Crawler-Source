#pragma once
#include <string>
#include <map>

class Object;

namespace Crawl
{
	struct Hall
	{
		int column, row;
		bool occupied = false;

		Object* object;
	};
	struct Column
	{
		std::map<int, Hall> row;
	};

	class Dungeon
	{
	public:
		// Room/Hall manipulation
		Hall* AddHall(int column, int row);
		void DeleteHall(int column, int row);

		// Tests
		bool IsOpen(int column, int row);
		bool CanMove(int fromColumn, int fromRow, int toColumn, int toRow);
	
	public: // but should be protected when not deving shit
		const int version = 1;
		std::map<int, Column> halls;
		
	public:
		void Save(std::string filename);
		void Load(std::string filename);
	};
}


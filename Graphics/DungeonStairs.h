#include "DungeonHelpers.h"
#include "glm.hpp"
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonStairs
	{
	public:
		glm::vec3 EvaluatePosition(float t);
		void BuildDefaultSpline();
	public:
		glm::ivec2 startPosition = { 0, 0 };
		glm::ivec2 endPosition = { 0,0 };
		FACING_INDEX directionStart = NORTH_INDEX;
		FACING_INDEX directionEnd = NORTH_INDEX;
		
		bool up = true;
		
		glm::vec3 startWorldPosition = {0,0,0 };
		glm::vec3 startOffset = { 0,0,0 };;
		glm::vec3 endWorldPosition = { 0,0,0 };;
		glm::vec3 endOffset = { 0,0,0 };;
	};

	static void to_json(ordered_json& j, const DungeonStairs& object)
	{
		j = {
			{"startPosition", object.startPosition},
			{"endPosition", object.endPosition},
			{"directionStart", object.directionStart},
			{"directionEnd", object.directionEnd},
			
			{"up", object.up},
			
			{"startWorldPosition", object.startWorldPosition},
			{"startOffset", object.startOffset},
			{"endWorldPosition", object.endWorldPosition},
			{"endOffset", object.endOffset},
		};
	}

	static void from_json(const ordered_json& j, DungeonStairs& object)
	{
		j.at("startPosition").get_to(object.startPosition);
		j.at("endPosition").get_to(object.endPosition);
		j.at("directionStart").get_to(object.directionStart);
		j.at("directionEnd").get_to(object.directionEnd);
		
		j.at("up").get_to(object.up);
		
		j.at("startWorldPosition").get_to(object.startWorldPosition);
		j.at("startOffset").get_to(object.startOffset);
		j.at("endWorldPosition").get_to(object.endWorldPosition);
		j.at("endOffset").get_to(object.endOffset);
	}
}
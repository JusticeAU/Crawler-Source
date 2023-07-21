#include "glm.hpp"
#include "serialisation.h"

using glm::ivec2;
class Object;

namespace Crawl
{

	class DungeonSpikes
	{
	public:
		~DungeonSpikes();
		ivec2 position = {0,0};
		bool disabled = false;
		Object* object = nullptr;

	private:
	};

	static void to_json(ordered_json& j, const DungeonSpikes& object)
	{
		j = { {"position", object.position } };
	}

	static void from_json(const ordered_json& j, DungeonSpikes& object)
	{
		j.at("position").get_to(object.position);
	}
}
#pragma once
#include <glm.hpp>
#include "serialisation.h"

class Object;
class ComponentRenderer;

namespace Crawl
{
	class DungeonDoor
	{
	public:
		~DungeonDoor();
		void Toggle();
		void Open(bool instant = false);
		void Close(bool instance = false);

		void UpdateVisuals(float delta);
		void UpdateTransforms(bool instant = false);

		void Interact();

		void MakeBarricaded();
		void RemoveBarricaded();
		
		void MakeLobbyDoor();
		void RemoveLobbyDoorMaterial();

		void PlaySFX(string sfx);

		unsigned int id = 0;
		glm::ivec2 position = { 0, 0 };
		int orientation = 0; // Use DIRECTION_MASK in DungeonHelpers.h
		bool isLobbyDoor = false;
		
		Object* object = nullptr;
		ComponentRenderer* renderer = nullptr;
		bool open = false;
		
		// Open/Close animations
		const float openEulerAngle = -120.0f;
		const float swingTime = 0.5f;
		float swingTimeCurrent = 0.0f;
		bool shouldSwing = false;
		string openSound = "crawler/sound/load/door/open_1.wav";
		string closeSound = "crawler/sound/load/door/close_1.wav";


		// Wobble Interaction
		bool shouldWobble = false;
		const float wobbleTime = 0.4f;
		float wobbleTimeCurrent = 0.0f;
		string wobbleSound = "crawler/sound/load/door/rattle_1.wav";
		
		// Barricade info
		bool isBarricaded = false;
		Object* objectBarricade = nullptr;
		string objectBarricadeJSON = "crawler/model/decoration_barricade.object";

		// Lobby alternative material
		const string lobbyAlternativeMaterial = "crawler/material/door_texture_doorframe_lobby.material";
		string doorOriginalMaterial = "";
	};

	static void to_json(ordered_json& j, const DungeonDoor& door)
	{
		j = { {"position", door.position}, {"id", door.id}, {"orientation", door.orientation}, {"open", door.open}, {"isBarricaded", door.isBarricaded } };
		if (door.isLobbyDoor) j["isLobbyDoor"] = true;
	}

	static void from_json(const ordered_json& j, DungeonDoor& door)
	{
		j.at("position").get_to(door.position);
		j.at("id").get_to(door.id);
		j.at("orientation").get_to(door.orientation);
		j.at("open").get_to(door.open);
		if (j.contains("isBarricaded")) j.at("isBarricaded").get_to(door.isBarricaded);
		if (j.contains("isLobbyDoor")) j.at("isLobbyDoor").get_to(door.isLobbyDoor);

	}
}
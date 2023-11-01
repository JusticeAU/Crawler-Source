#pragma once
#include <string>

class Texture;

using std::string;

namespace Crawl
{
	class DungeonMenuButton
	{
	public:
		DungeonMenuButton(string name, string texturePath);
		bool Update(float delta);
		void SetActive(bool active = true);
		string name = "";
		Texture* tex = nullptr;
		
		bool isHovered = false;

		float offsetPositionMax = 30.0f;
		float offsetTimeTotal = 0.15;
		float alphaCurrent = 0.0f;
		float unselectedAlpha = 0.5f;

		float t = 0.0f;
		float offsetPositionCurrent = 1.0f;
		float offsetTimeCurrent = 0.0f;

		string hoverTexturePath = "crawler/texture/gui/menu/hover.tga";
		Texture* hoverTexture = nullptr;

		bool isFadingOut = false;
		float fadeOutTime = 0.5f;
		float fadeOutCurrent = 0.0f;
		float fadeAlphaStart = 0.0f;
	};
}

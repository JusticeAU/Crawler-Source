#pragma once
#include <string>
#include <functional>

class Texture;

using std::string;

namespace Crawl
{
	class DungeonMenu;
	class DungeonMenuButton
	{
	public:
		DungeonMenuButton(string name, string texturePath);
		static void InitialiseCheckmarkTextures();

		void BindMenuFunction(void (DungeonMenu::*&& _Func)(), DungeonMenu*&& _Args) { menuFunction = std::bind(_Func, _Args); };
		void BindBoolPointer(bool* toggle) { boolMonitor = toggle; };
		void Activate() { menuFunction(); };

		void Hover();
		
		void Update(float delta);
		
		bool IsMouseOver() { return isMouseOver; };
		void SetEnabled(bool enabled = true);
		string name = "";
		Texture* tex = nullptr;
		std::function<void()> menuFunction;
		bool* boolMonitor;
		static Texture* boolCheckedTex;
		static Texture* boolUncheckedTex;

		
		bool isMouseOver = false;
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


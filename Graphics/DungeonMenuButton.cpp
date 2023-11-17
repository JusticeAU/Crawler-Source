#include "DungeonMenuButton.h"
#include "TextureManager.h"
#include "imgui.h"
#include "glm.hpp"
#include "LogUtils.h"
#include "MathUtils.h"
#include "DungeonMenu.h"

Texture* Crawl::DungeonMenuButton::boolCheckedTex = nullptr;
Texture* Crawl::DungeonMenuButton::boolUncheckedTex = nullptr;


Crawl::DungeonMenuButton::DungeonMenuButton(string name, string texturePath)
{
	this->name = name;
	tex = TextureManager::GetTexture(texturePath);
	hoverTexture = TextureManager::GetTexture(hoverTexturePath);
}

void Crawl::DungeonMenuButton::InitialiseCheckmarkTextures()
{
	boolCheckedTex = TextureManager::GetTexture("crawler/texture/gui/menu/checkbox_checked.tga");
	boolUncheckedTex = TextureManager::GetTexture("crawler/texture/gui/menu/checkbox_empty.tga");;
}

void Crawl::DungeonMenuButton::Hover()
{
	isHovered = true;
}

void Crawl::DungeonMenuButton::Update(float delta)
{
	alphaCurrent = MathUtils::Lerp(unselectedAlpha, 1.0f, t);
	float alpha = !isFadingOut ? alphaCurrent : MathUtils::Lerp(fadeAlphaStart, 0, fadeOutCurrent / fadeOutTime);

	bool spacerClicked = false;
	bool spacerHovered = false;

	bool buttonClicked = false;
	bool buttonHovered = false;

	if (isFadingOut)
		fadeOutCurrent += delta;

	if (isHovered && !isFadingOut)
	{
		ImGui::Image(
			(ImTextureID)hoverTexture->texID,
			{ (float)hoverTexture->width * t, (float)hoverTexture->height },
			{ 0,1 },
			{ 1,0 },
			{1,1,1,t});
		ImGui::SameLine(8);
	}

	string invisbut = name + "invi";
	spacerClicked = ImGui::InvisibleButton(invisbut.c_str(), {(float)offsetPositionCurrent, (float)tex->height}) && !isFadingOut;
	if (ImGui::IsItemHovered()) spacerHovered = true;
	ImGui::SameLine(0.0f,0.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));


	buttonClicked = ImGui::ImageButton(
		name.c_str(),
		(ImTextureID)tex->texID,
		{ (float)tex->width, (float)tex->height },
		{ 0,1 },
		{ 1,0 },
		{ 0,0,0,0 },
		{ 1,1,1,alpha }) && !isFadingOut;
	ImGui::PopStyleColor(3);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped)) buttonHovered = true;


	if (boolMonitor != nullptr)
	{
		ImGui::SameLine(550.0f, 0.0f);
		if (*boolMonitor)
			DungeonMenu::DrawImage(boolCheckedTex, alpha);
		else
			DungeonMenu::DrawImage(boolUncheckedTex, alpha);

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped)) buttonHovered = true;
	}

	// Update dynamic indentation
	isMouseOver = spacerHovered || buttonHovered;
	if (isHovered) offsetTimeCurrent = glm::min(offsetTimeCurrent + delta, offsetTimeTotal);
	else offsetTimeCurrent = glm::max(offsetTimeCurrent - delta, 0.0f);

	t = offsetTimeCurrent / offsetTimeTotal;
	offsetPositionCurrent = glm::max(offsetPositionMax * t, 1.0f);
	isHovered = false;

	return;
}

void Crawl::DungeonMenuButton::SetEnabled(bool enabled)
{
	isFadingOut = !enabled;
	if(isFadingOut) fadeAlphaStart = alphaCurrent;
}
#pragma once
#include "Graphics.h"
#include "serialisation.h"
#include <string>

using glm::vec3;
using std::string;

class Texture;
class ShaderProgram;

class Material
{
public:
	void DrawGUI();
	void LoadTextures();
	void SaveToFile();

	bool loaded = false;
	bool isInGameData = false;

	bool Audit_referenced = false;

	string name;
	string filePath;

	vec3 Ka = { 0.0f, 0.0f, 0.0f }; // ambient colour of the surface
	vec3 Kd = { 1.0f, 1.0f, 1.0f };; // diffuse colour of the surface
	vec3 Ks = { 1.0f, 1.0f, 1.0f };; // specular colour of the surface
	float specularPower = 20.0f; // tightness of specular highlights

	Texture* mapKd = nullptr; // Diffuse texture map
	string mapKdName = "";
	Texture* mapKs = nullptr; // specular texture map
	string mapKsName = "";
	Texture* mapBump = nullptr; // normal map
	string mapBumpName = "";

	ShaderProgram* shader = nullptr;
	string shaderName = "";
	ShaderProgram* shaderSkinned = nullptr;
	string shaderSkinnedName = "";

	enum class BlendMode
	{
		Opaque,
		AlphaCutoff,
		Transparent,
		Count
	};
	static string blendModeStrings[3];

	BlendMode blendMode = BlendMode::Opaque;

	// PBR
	bool isPBR = false;
	Texture* albedoMap		= nullptr;
	Texture* normalMap		= nullptr;
	Texture* metallicMap	= nullptr;
	Texture* roughnessMap	= nullptr;
	Texture* aoMap = nullptr;
	Texture* emissiveMap = nullptr;

	string albedoMapName	= "";
	string normalMapName	= "";
	string metallicMapName	= "";
	string roughnessMapName = "";
	string aoMapName		= "";
	string emissiveMapName = "";
};

extern void to_json(nlohmann::ordered_json& j, const Material& mat);
extern void from_json(const nlohmann::ordered_json& j, Material& mat);
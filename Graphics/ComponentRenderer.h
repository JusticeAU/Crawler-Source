#pragma once
#include "Component.h"

#include <string>

class Model;
class ShaderProgram;
class Texture;
class Material;

using std::string;

class ComponentRenderer : public Component
{
public:
	ComponentRenderer(Object* parent) : Component("Renderer", Component_Renderer, parent) {};
	ComponentRenderer(Object* parent, std::istream& istream);

	void Draw() override;

	void DrawGUI() override;

	void Write(std::ostream& ostream) override;

	virtual void OnParentChange() override;

	void BindShader();
	void BindMatricies();
	void SetUniforms();
	void ApplyTexture();
	void ApplyMaterials();
	void DrawModel();
public:
	Model* model = nullptr;
	
	ShaderProgram* shader = nullptr;
	string shaderName = "";
	
	Texture* texture = nullptr;
	string textureName = "";
	
	Material* material = nullptr;
	string materialName = "";
};
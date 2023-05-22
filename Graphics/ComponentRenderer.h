#pragma once
#include "Component.h"

#include <string>

class Camera;
class Model;
class ShaderProgram;
class Texture;
class Material;
class FrameBuffer;

using std::string;

class ComponentRenderer : public Component
{
public:
	ComponentRenderer(Object* parent) : Component("Renderer", Component_Renderer, parent) {};
	ComponentRenderer(Object* parent, std::istream& istream);

	void Draw(mat4 pv, vec3 position, DrawMode mode) override;

	void DrawGUI() override;

	void Write(std::ostream& ostream) override;

	virtual void OnParentChange() override;

	void BindShader();
	void BindMatricies(mat4 pv, vec3 position);
	void SetUniforms();
	void ApplyTexture();
	void ApplyMaterials();
	void DrawModel();

	Component* Clone(Object* parent);
public:
	Model* model = nullptr;
	
	ShaderProgram* shader = nullptr;
	string shaderName = "";
	
	Texture* texture = nullptr;
	string textureName = "";
	
	Material* material = nullptr;
	string materialName = "";

	FrameBuffer* frameBuffer = nullptr;
	string frameBufferName = "";

	bool castsShadows = true;
	bool receivesShadows = true;
	float shadowBias = 0.005f;
};
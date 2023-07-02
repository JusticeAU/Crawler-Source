#pragma once
#include "Component.h"

#include <string>
#include <vector>

class Camera;
class Model;
class ShaderProgram;
class Texture;
class Material;
class FrameBuffer;

using std::string;
using std::vector;

class ComponentRenderer : public Component
{
public:
	ComponentRenderer(Object* parent) : Component("Renderer", Component_Renderer, parent) {};
	ComponentRenderer(Object* parent, nlohmann::ordered_json);

	void Draw(mat4 pv, vec3 position, DrawMode mode) override;

	void DrawGUI() override;

	virtual void OnParentChange() override;

	void BindShader();
	void BindMatricies(mat4 pv, vec3 position);
	void SetUniforms();
	void ApplyMaterials();
	void DrawModel();

	Component* Clone(Object* parent);
public:
	Model* model = nullptr;
	
	vector<Material*> materialArray;
	Material* material = nullptr;

	FrameBuffer* frameBuffer = nullptr;
	string frameBufferName = "";

	bool castsShadows = true;
	bool receivesShadows = true;
	float shadowBias = 0.005f;
};
#pragma once
#include "Graphics.h"
#include "Component.h"
#include "serialisation.h"

#include <vector>
#include <string>

using glm::vec3;
using glm::mat4;
using std::vector;
using std::string;

class Camera;
class Model;
class Texture;
class ShaderProgram;
class Material;
class UniformBuffer;

// Primary container for objects in the application. Contains all model, texture, shader, material and animation information.
// Ideally this should all be split out in to a component system, similar to Unity where this only holds Transform information.

class Object
{
public:
	Object(int objectID, string name = "New Object");
	~Object();
	unsigned int id = 0;
	
	Object* parent = nullptr;
	vector<Object*> children;
	
	bool dirtyTransform = true;
	vec3 localPosition;
	vec3 localRotation;
	vec3 localScale;
	mat4 localTransform;
	mat4 transform;
	
	bool markedForDeletion = false;

	string objectName;

	vector<Component*> components;

	void Update(float delta);
	void Draw(mat4 pv, vec3 position, Component::DrawMode mode);

	void SetLocalPosition(vec3 localPos);
	void SetLocalRotation(vec3 euler);
	void SetLocalScale(vec3 localScale);
	void AddLocalPosition(vec3 localPos);
	void AddLocalRotation(vec3 euler);
	void AddLocalScale(vec3 localScale);

	void DrawGUI();
	void DrawGUISimple();
	
	void AddChild(Object* child);
	void CleanUpChildren();
	void DeleteAllChildren();

	vec3 GetWorldSpacePosition() { return { transform[3][0], transform[3][1], transform[3][2] }; } // Column-Major

	void LoadFromJSON(nlohmann::ordered_json j);

	Component* GetComponent(ComponentType type);

	void RefreshComponents();
	void RecalculateTransforms();

	Object* FindObjectWithID(unsigned int id);
};

extern void to_json(nlohmann::ordered_json& j, const Object& object);
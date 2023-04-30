#pragma once
#include "Graphics.h"
#include "Component.h"

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
	unsigned int id;
	
	Object* parent = nullptr;
	vector<Object*> children;
	
	vec3 eulerRotation; // Keep this around to avoid Imguizmo decomposing it and potentially flipping what is 'up'

	bool dirtyTransform = true;
	mat4 transform;
	mat4 localTransform;
	
	bool markedForDeletion = false;

	string objectName;

	vector<Component*> components;

	// Debug helpers
	bool spin = false;
	float spinSpeed = 10.0f;

	void Update(float delta);
	void Draw(mat4 pv, vec3 position, Component::DrawMode mode);
	void DrawGUI();
	void DrawGUISimple();
	
	void AddChild(Object* child);
	void CleanUpChildren();
	void DeleteAllChildren();

	vec3 GetWorldSpacePosition() { return { transform[3][0], transform[3][1], transform[3][2] }; } // Column-Major

	void Write(std::ostream& out);
	void Read(std::istream& in);

	Component* GetComponent(ComponentType type);

	void RefreshComponents();
	void RecalculateTransforms();
};
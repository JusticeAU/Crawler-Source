#pragma once
#include "Graphics.h"
#include <vector>
#include <string>

using glm::vec3;
using glm::mat4;
using std::vector;
using std::string;

class Component;
enum ComponentType;
class Model;
class Texture;
class ShaderProgram;
class Material;
class UniformBuffer;

const int MAX_BONES = 100;

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
	
	vec3 localPosition;
	vec3 localRotation;
	vec3 localScale;
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
	void Draw();
	void DrawGUI();
	void DrawGUISimple();
	
	void AddChild(Object* child);
	void CleanUpChildren();
	void DeleteAllChildren();

	void Write(std::ostream& out);
	void Read(std::istream& in);

	Component* GetComponent(ComponentType type);

	void UpdateBoneMatrixBuffer(float frameTime);
	void ProcessNode(float frameTime, int animationIndex, Object* node, mat4 accumulated);
};
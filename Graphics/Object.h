#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "Graphics.h"
#include <string>

using glm::vec3;
using glm::mat4;
using std::string;

class ShaderProgram;

class Object
{
public:
	Object(int objectID);
	
	unsigned int id;
	
	Object* parent = nullptr;;
	vector<Object*> children;
	
	vec3 localPosition;
	vec3 localRotation;
	vec3 localScale;
	mat4 transform;
	
	bool markedForDeletion = false;

	ShaderProgram* shader;
	Mesh* mesh;
	string meshName;

	Texture* texture;
	string textureName;

	void Update(float delta);
	void Draw();
	void DrawGUI();
	
	void AddChild(Object* child);
	void CleanUpChildren();
	void DeleteAllChildren();
};
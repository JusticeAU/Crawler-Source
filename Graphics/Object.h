#pragma once
#include "Mesh.h"
#include "Graphics.h"
#include <string>

using glm::vec3;
using glm::mat4;
using std::string;

class ShaderProgram;

class Object
{
public:
	Object();
	Object* parent = nullptr;;
	vector<Object*> children;
	
	vec3 localPosition;
	vec3 worldPosition;
	mat4 translationMat;
	vec3 localRotation;
	mat4 rotationMat;
	bool markedForDeletion = false;
	unsigned int id;

	ShaderProgram* shader;

	Mesh* mesh;

	void Update(float delta);
	void Draw();
	void DrawGUI();
	void AddChild(Object* child);
};
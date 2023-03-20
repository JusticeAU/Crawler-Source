#pragma once
#include "Mesh.h"
#include "Graphics.h"
#include <string>

using glm::vec3;
using std::string;

class ShaderProgram;

class Object
{
public:
	Object();
	
	vec3 position;
	vec3 rotation;
	bool markedForDeletion = false;
	unsigned int id;

	ShaderProgram* shader;

	Mesh* mesh;

	void Update(float delta);
	void Draw();
};
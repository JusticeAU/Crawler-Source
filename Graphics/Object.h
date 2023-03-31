#pragma once
#include "Graphics.h"
#include <vector>
#include <string>

using glm::vec3;
using glm::mat4;
using std::vector;
using std::string;

class Mesh;
class Texture;
class ShaderProgram;
class Material;

class Object
{
public:
	Object(int objectID, string name = "New Object");
	unsigned int id;
	
	Object* parent = nullptr;
	vector<Object*> children;
	
	vec3 localPosition;
	vec3 localRotation;
	vec3 localScale;
	mat4 transform;
	mat4 localTransform;
	
	bool markedForDeletion = false;

	string objectName;
	
	Mesh* mesh;
	string meshName;

	Texture* texture;
	string textureName;

	ShaderProgram* shader;
	string shaderName;

	// "Material" Properties - to be moved to a separate class
	Material* material;
	string materialName;

	int selectedBone = 0;

	void Update(float delta);
	void Draw();
	void DrawGUI();
	void DrawGUISimple();
	
	void AddChild(Object* child);
	void CleanUpChildren();
	void DeleteAllChildren();

	void Write(std::ostream& out);
	void Read(std::istream& in);
};
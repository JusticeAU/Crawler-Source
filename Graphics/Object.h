#pragma once
#include "Graphics.h"
#include <vector>
#include <string>

using glm::vec3;
using glm::mat4;
using std::vector;
using std::string;

class Model;
class Texture;
class ShaderProgram;
class Material;

const int MAX_BONES = 100;

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
	mat4 transform;
	mat4 localTransform;
	
	bool markedForDeletion = false;

	string objectName;
	
	Model* model;
	string modelName;

	Texture* texture;
	string textureName;

	ShaderProgram* shader;
	string shaderName;

	// "Material" Properties - to be moved to a separate class
	Material* material;
	string materialName;

	// Animation state
	int selectedBone = 0;
	int selectedAnimation = 0;
	string animationName = "";
	bool loopAnimation = true;
	bool playAnimation = true;
	float animationSpeed = 1.0f;
	float animationTime = 0.0f;
	int selectedFrame = 0;
	mat4* boneTransforms;



	void Update(float delta);
	void Draw();
	void DrawGUI();
	void DrawGUISimple();
	
	void AddChild(Object* child);
	void CleanUpChildren();
	void DeleteAllChildren();

	void Write(std::ostream& out);
	void Read(std::istream& in);

	void UpdateBoneMatrixBuffer(float frameTime);
	void ProcessNode(float frameTime, int animationIndex, Object* node, mat4 accumulated);
};
#pragma once
#include "Graphics.h"
#include "FileUtils.h"
#include <string>

class Camera;
class Object;

using std::string;
using glm::mat4;
using glm::vec3;

enum ComponentType
{
	Component_Model,
	Component_Animator,
	Component_Renderer,
	Component_SkinnedRenderer,
	Component_Material,
	Component_Light,
	Component_Camera,
	Component_FPSTest,
	Component_AnimationBlender
};

class Component
{
public:
	virtual ~Component() {};

	virtual void Update(float deltaTime) {};
	virtual void Draw(mat4 pv, vec3 position, bool picking) {};

	string GetName() { return componentName; };
	ComponentType GetType() { return componentType; };
	virtual void DrawGUI() {};

	virtual void Write(std::ostream& ostream) {};

	void AnnounceChange();
	virtual void OnParentChange() {};

	const Object* GetComponentParentObject() { return componentParent; }

	bool markedForDeletion = false;

protected:
	Component(string name, ComponentType type, Object* parent) : componentName(name), componentType(type), componentParent(parent) {};
	Object* componentParent;
	ComponentType componentType;
	string componentName;
};
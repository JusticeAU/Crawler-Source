#pragma once
#include "Graphics.h"
#include "FileUtils.h"
#include <string>

class Object;

using std::string;

enum ComponentType
{
	Component_Model,
	Component_Animator,
	Component_Renderer,
	Component_SkinnedRenderer,
	Component_Material,
	Component_Light,
	Component_Camera
};

class Component
{
public:
	virtual void Update(float deltaTime) {};
	virtual void Draw() {};

	string GetName() { return componentName; };
	ComponentType GetType() { return componentType; };
	virtual void DrawGUI() {};

	virtual void Write(std::ostream& ostream) {};

	void AnnounceChange();
	virtual void OnParentChange() {};

protected:
	Component(string name, ComponentType type, Object* parent) : componentName(name), componentType(type), componentParent(parent) {};
	Object* componentParent;
	ComponentType componentType;
	string componentName;
};
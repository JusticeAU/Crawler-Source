#pragma once
#include "Graphics.h"
#include "FileUtils.h"
#include "serialisation.h"
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
	Component_AnimationBlender,
	Component_AudioSource
};

class Component
{
public:
	enum class DrawMode
	{
		Standard,
		ObjectPicking,
		ShadowMapping,
		SSAOgBuffer,
		SSAOColourPass
	};
	virtual ~Component() {};

	virtual void Update(float deltaTime) {};
	virtual void Draw(mat4 pv, vec3 position, DrawMode mode) {};

	string GetName() { return componentName; };
	ComponentType GetType() { return componentType; };
	virtual void DrawGUI() {};

	void AnnounceChange();
	virtual void OnParentChange() {};

	Object* GetComponentParentObject() { return componentParent; }

	virtual Component* Clone(Object* parent) { return nullptr; }

	bool markedForDeletion = false;

protected:
	Component(string name, ComponentType type, Object* parent) : componentName(name), componentType(type), componentParent(parent) {};
	Object* componentParent;
	ComponentType componentType;
	string componentName;
};
#pragma once
#include "Component.h"
#include "ComponentModel.h"
#include "ComponentAnimator.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "ComponentAudioSource.h"
#include "ComponentLightPoint.h"
#include "ComponentBillboard.h"
#include "ComponentParticleSystem.h"
#include "PostProcess.h"

#include "serialisation.h"

#include <vector>
#include <string>

using std::string;
using std::vector;

class Object;

static class ComponentFactory
{
public:
	static void Init();
	static unsigned int GetComponentCount() { return components.size(); };
	static string GetComponentName(unsigned int i) { return components[i]; };

	static Component* NewComponent(Object* parent, ComponentType type);
	static Component* ReadComponentJSON(Object* parent, nlohmann::ordered_json j);
protected:
	static vector<string> components;
};
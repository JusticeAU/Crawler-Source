#pragma once
#include "ComponentModel.h"
#include "ComponentAnimator.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "ComponentFPSTest.h"
#include "ComponentAnimationBlender.h"
#include "ComponentAudioSource.h"
#include "PostProcess.h"
#include "serialisation.h"

#include <vector>
#include <string>

using std::string;
using std::vector;

static class ComponentFactory
{
public:
	static void Init();
	static unsigned int GetComponentCount() { return components.size(); };
	static string GetComponentName(unsigned int i) { return components[i]; };

	static Component* NewComponent(Object* parent, int i);
	static Component* ReadComponentJSON(Object* parent, nlohmann::ordered_json j);
protected:
	static vector<string> components;
};
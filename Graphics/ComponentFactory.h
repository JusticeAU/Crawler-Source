#pragma once
#include "ComponentModel.h"
#include "ComponentAnimator.h"
#include "ComponentRenderer.h"
#include "ComponentSkinnedRenderer.h"
#include "ComponentCamera.h"
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
	static Component* ReadComponent(Object* parent, std::istream& istream, ComponentType type);
protected:
	static vector<string> components;
};
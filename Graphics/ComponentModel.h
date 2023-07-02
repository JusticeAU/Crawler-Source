#pragma once
#include "Component.h"

#include <string>

class Model;

using std::string;

class ComponentModel : public Component
{
public:
	ComponentModel(Object* parent) : Component("Model", Component_Model, parent) {};
	ComponentModel(Object* parent, nlohmann::ordered_json j);
	void DrawGUI() override;

	Component* Clone(Object* parent);

	string modelName = "";
	Model* model = nullptr;
};
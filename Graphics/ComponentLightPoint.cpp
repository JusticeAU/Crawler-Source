#include "ComponentLightPoint.h"
#include "Scene.h"

ComponentLightPoint::ComponentLightPoint(Object* parent) : Component("Point Light", Component_LightPoint, parent)
{
	//parent->components.push_back(this);
	Scene::s_instance->AddPointLight(this);
}

ComponentLightPoint::ComponentLightPoint(Object* parent, nlohmann::ordered_json j) : ComponentLightPoint(parent)
{
	j.at("colour").get_to(colour);
	j.at("intensity").get_to(intensity);
}

ComponentLightPoint::~ComponentLightPoint()
{
	Scene::s_instance->RemovePointLight(this);
}

void ComponentLightPoint::DrawGUI()
{
	ImGui::ColorEdit3("Colour", &colour.x);
	ImGui::DragFloat("Intensity", &intensity);
}

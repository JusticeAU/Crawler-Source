#include "Object.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"

#include "ShaderProgram.h"
#include "UniformBuffer.h"
#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include <string>

#include "ComponentFactory.h";

#include "Window.h"

#include "serialisation.h"

using std::to_string;

Object::Object(int objectID, string name)
{
	id = objectID;

	localPosition = { 0,0,0 };
	localRotation = { 0,0,0 };
	localScale = { 1,1,1 };

	localTransform = mat4(1);
	transform = mat4(1);

	objectName = name;
}

Object::~Object()
{
	for (auto child = children.begin(); child != children.end(); child++)
	{
		auto c = *child;
		delete c;
	}

	for (auto component = components.begin(); component != components.end(); component++)
	{
		auto c = *component;
		delete c;
	}

}

void Object::Update(float delta)
{

	if (dirtyTransform) // Our transform has changed. Update it and our childrens transforms.
	{
		RecalculateTransforms();
		dirtyTransform = false; // clear our dirty flag
	}

	// Update all components
	for (auto component : components)
		component->Update(delta);

	// Update all children recursively.
	for (auto c : children)
		c->Update(delta);
}

void Object::Draw(mat4 pv, vec3 position, Component::DrawMode mode)
{
	for (auto component : components)
		component->Draw(pv, position, mode);

	for (auto c : children)
		c->Draw(pv, position, mode);
}

void Object::SetLocalPosition(vec3 pos)
{
	localPosition = pos;
	dirtyTransform = true;
}

void Object::SetLocalRotation(vec3 euler)
{
	localRotation = euler;
	dirtyTransform = true;
}

void Object::SetLocalRotationX(float x)
{
	localRotation.x = x;
	dirtyTransform = true;
}

void Object::SetLocalRotationY(float y)
{
	localRotation.y = y;
	dirtyTransform = true;
}

void Object::SetLocalRotationZ(float z)
{
	localRotation.z = z;
	dirtyTransform = true;
}

void Object::SetLocalScale(vec3 scale)
{
	localScale = scale;
	dirtyTransform = true;
}

void Object::SetLocalScale(float uniformScale)
{
	localScale = vec3(uniformScale);
	dirtyTransform = true;
}

void Object::AddLocalPosition(vec3 pos)
{
	localPosition += pos;
	dirtyTransform = true;
}

void Object::AddLocalRotation(vec3 euler)
{
	localRotation += euler;
	dirtyTransform = true;
}

void Object::AddLocalScale(vec3 scale)
{
	localScale += scale;
	dirtyTransform = true;
}


// Draws all Imgui data for an object in the scene window.
void Object::DrawGUI()
{
	ImGui::PushID(id);
	string headerTitle = objectName + " (" + to_string(id) + ")";
	if (ImGui::CollapsingHeader(headerTitle.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
	{
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
			markedForDeletion = true;
		
		ImGui::SameLine();
		if (ImGui::Button("SaveJSON"))
			SaveObjectToJSON();


		ImGui::Indent();
		string newName = objectName;
		if (ImGui::InputText(objectName.c_str(), &newName, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			objectName = newName;
		}
		
		if (ImGui::CollapsingHeader("Transform"))
		{
			ImGui::Indent();
			if (ImGui::DragFloat3("Position", &localPosition[0]))
				dirtyTransform = true;

			if(ImGui::SliderFloat3("Rotation", &localRotation[0], -180, 180))
				dirtyTransform = true;

			if(ImGui::DragFloat3("Scale", &localScale[0]))
				dirtyTransform = true;
		}

		// Draw Components.
		if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SameLine();
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("popup_add_componenent");

			ImGui::Indent();
			for (int i = 0; i < components.size(); i++)
			{
				ImGui::PushID(i);
				string componentName = components[i]->GetName();
				if (ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
				{
					ImGui::SameLine();
					if (ImGui::Button("Delete"))
					{
						Component* comp = components[i];
						delete comp;
						components.erase(components.begin() + i);
						i--;
					}
					else
						components[i]->DrawGUI();
				}
				else
				{
					ImGui::SameLine();
					if (ImGui::Button("Delete"))
					{
						Component* comp = components[i];
						comp->markedForDeletion = true;
						delete comp;
						components.erase(components.begin() + i);
						i--;
					}
				}
				ImGui::PopID();
			}
			ImGui::Unindent();
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("popup_add_componenent");
		}

		// Draw Add Component Popup if requested.
		if (ImGui::BeginPopup("popup_add_componenent"))
		{
			ImGui::SameLine();
			ImGui::SeparatorText("Component");
			for (int i = 0; i < ComponentFactory::GetComponentCount(); i++)
			{
				if (ImGui::Selectable(ComponentFactory::GetComponentName(i).c_str()))
				{
					components.push_back(ComponentFactory::NewComponent(this,i));
					for (auto component : components)
						component->OnParentChange();
				}
			}
			ImGui::EndPopup();
		}

		// Draw children information.
		if (ImGui::CollapsingHeader("Children", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::SameLine();
			if (ImGui::Button("Add Child"))
			{
				Object* spawn = Scene::CreateObject(this);
				spawn->Update(0.0f);
			}
			ImGui::Indent();
			for (auto c : children)
				c->DrawGUI();
			ImGui::Unindent();
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Add Child"))
			{
				Object* spawn = Scene::CreateObject(this);
				spawn->Update(0.0f);
			}
		}
		ImGui::Unindent();
	}
	else
	{
		ImGui::SameLine();
		if (ImGui::Button("Duplicate"))
			Scene::DuplicateObject(this, parent);
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
			markedForDeletion = true;
		
	}

	ImGui::PopID();
}

// Used to display the bone hierarchy (which is made up of objects repurposed) without dispalying a bunch of other noisy data that's not used in this context.
void Object::DrawGUISimple()
{
	ImGui::PushID(id);
	string objectStr = objectName;
	if (ImGui::TreeNode(objectStr.c_str()))
	{
		/*if (ImGui::CollapsingHeader("Transform"))
		{
			vec3 localPosition, localRotation, localScale;
			ImGuizmo::DecomposeMatrixToComponents((float*)&localTransform, (float*)&localPosition, (float*)&localRotation, (float*)&localScale);
			if (ImGui::DragFloat3("Position", &localPosition[0]))
				dirtyTransform = true;

			if (ImGui::SliderFloat3("Rotation", &eulerRotation[0], -180, 180))
				dirtyTransform = true;

			if (ImGui::DragFloat3("Scale", &localScale[0]))
				dirtyTransform = true;

			if (dirtyTransform)
				ImGuizmo::RecomposeMatrixFromComponents((float*)&localPosition, (float*)&eulerRotation, (float*)&localScale, (float*)&localTransform);
		}*/
		//ImGui::Indent();
		/*ImGui::BeginDisabled();
		ImGui::DragFloat4("", &parent->transform[0][0]);
		ImGui::DragFloat4("", &parent->transform[1][0]);
		ImGui::DragFloat4("", &parent->transform[2][0]);
		ImGui::DragFloat4("", &parent->transform[3][0]);
		ImGui::EndDisabled();*/

		for (auto c : children)
			c->DrawGUISimple();

		//ImGui::Unindent();

		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Object::AddChild(Object* child)
{
	child->parent = this;
	children.push_back(child);
}

void Object::CleanUpComponents()
{
	for (int i = 0; i < components.size(); i++)
	{
		if (components[i]->markedForDeletion)
		{
			components.erase(components.begin() + i);
			i--;
		}
	}
}

void Object::CleanUpChildren()
{
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->CleanUpComponents();
		if (children[i]->markedForDeletion)
		{
			children[i]->DeleteAllChildren();
			children.erase(children.begin() + i);
			i--;
		}
		else
			children[i]->CleanUpChildren();
	}
}

void Object::DeleteAllChildren()
{
	for (auto c : children)
		c->DeleteAllChildren();

	children.clear();
}

void Object::LoadFromJSON(nlohmann::ordered_json j)
{
	j.at("name").get_to(objectName);

	// Process Transform
	j.at("transform").at("position").get_to(localPosition);
	j.at("transform").at("rotation").get_to(localRotation);
	j.at("transform").at("scale").get_to(localScale);
	ImGuizmo::RecomposeMatrixFromComponents((float*)&localPosition, (float*)&localRotation, (float*)&localScale, (float*)&localTransform);

	// Process Components
	ordered_json componentsJSON = j["components"];
	if (componentsJSON.is_null() == false)
	{
		for (auto it = componentsJSON.begin(); it != componentsJSON.end(); it++)
			components.push_back(ComponentFactory::ReadComponentJSON(this, it.value()));
	}

	// Have all components run their OnChange function to init/refresh data
	for (auto component : components)
		component->OnParentChange();

	// Process Children
	ordered_json childrenJSON = j["children"];
	if (childrenJSON.is_null() == false)
	{
		for (auto it = childrenJSON.begin(); it != childrenJSON.end(); it++)
		{
			auto o = Scene::CreateObject(this);
			o->LoadFromJSON(it.value());
		}
	}
}

Component* Object::GetComponent(ComponentType type)
{
	for (auto component : components)
	{
		if (component->GetType() == type && !component->markedForDeletion)
			return component;
	}

	return nullptr;
}

void Object::RefreshComponents()
{
	for (auto c : components)
		c->OnParentChange();

	for (auto c : children)
		c->RefreshComponents();
}

void Object::RecalculateTransforms()
{
	ImGuizmo::RecomposeMatrixFromComponents((float*)&localPosition, (float*)&localRotation, (float*)&localScale, (float*)&localTransform);

	// Update world transform based on our parent.
	if (parent)
		transform = parent->transform * localTransform;
	else
		transform = localTransform;

	for (auto c : children)
		c->RecalculateTransforms();
}

Object* Object::FindObjectWithID(unsigned int id)
{
	if (id == this->id) return this;
	else
	{
		for (auto c : children)
			if (c->FindObjectWithID(id) != nullptr) return c;
	}

	return nullptr;
}

Object* Object::FindObjectWithName(string objectName)
{
	for (auto& o : children)
		if (o->objectName == objectName) return o;

	for (auto& o : children)
		o->FindObjectWithName(objectName);

	return nullptr;
}

// Just for dumping to disk for easy templating
void Object::SaveObjectToJSON()
{
	string filename = "something.object";
	ordered_json object = *this;
	WriteJSONToDisk(filename, object);
}

void to_json(nlohmann::ordered_json& j, const Object& object)
{
	j["name"] = object.objectName;

	// Process Transform
	ordered_json transformJSON;
	transformJSON["position"] = object.localPosition;
	transformJSON["rotation"] = object.localRotation;
	transformJSON["scale"] = object.localScale;
	j["transform"] = transformJSON;

	// Process Components
	ordered_json componentsJSON;
	for (int i = 0; i < object.components.size(); i++)
	{
		ordered_json c;
		switch (object.components[i]->GetType())
		{
		case ComponentType::Component_Model:
		{
			c["type"] = "Model";
			ComponentModel* cm = (ComponentModel*)object.components[i];
			c["model"] = cm->modelName;
			break;
		}
		case ComponentType::Component_Renderer:
		{
			c["type"] = "Renderer";
			ComponentRenderer* cr = (ComponentRenderer*)object.components[i];
			ordered_json matsJSON;
			for (int m = 0; m < cr->materialArray.size(); m++)
			{
				if (cr->materialArray[m])
					matsJSON.push_back(cr->materialArray[m]->name);
				else
					matsJSON.push_back("NULL");
			}
			c["materials"] = matsJSON;
			c["frameBuffer"] = cr->frameBufferName;
			c["receivesShadows"] = cr->receivesShadows;
			c["castsShadows"] = cr->castsShadows;
			break;
		}
		case ComponentType::Component_SkinnedRenderer:
		{
			c["type"] = "SkinnedRenderer";
			c["type"] = "Renderer";
			ComponentRenderer* cr = (ComponentRenderer*)object.components[i];
			ordered_json matsJSON;
			for (int m = 0; m < cr->materialArray.size(); m++)
			{
				if (cr->materialArray[m])
					matsJSON.push_back(cr->materialArray[m]->name);
				else
					matsJSON.push_back("NULL");
			}
			c["materials"] = matsJSON;
			c["frameBuffer"] = cr->frameBufferName;
			c["receivesShadows"] = cr->receivesShadows;
			c["castsShadows"] = cr->castsShadows;
			break;
		}
		case ComponentType::Component_Animator:
		{
			c["type"] = "Animator";
			ComponentAnimator* ca = (ComponentAnimator*)object.components[i];
			c["animationName"] = "Not currently storing!";
			break;
		}

		case ComponentType::Component_AudioSource:
		{
			c["type"] = "AudioSource";
			// TO DO
			break;
		}

		case ComponentType::Component_Camera:
		{
			c["type"] = "Camera";
			ComponentCamera* cc = (ComponentCamera*)object.components[i];
			c["nearClip"] = cc->nearClip;
			c["farClip"] = cc->farClip;
			c["fieldOfView"] = cc->fieldOfView;
			c["aspect"] = cc->aspect;

			ordered_json ppJSON;
			for (int pp = 0; pp < cc->m_postProcessStack.size(); pp++)
			{
				PostProcess* post = cc->m_postProcessStack[pp];
				ppJSON.push_back(post->GetShaderName());
			}

			c["postProcess"] = ppJSON;
			break;
		}
		}

		componentsJSON.push_back(c);
	}
	j["components"] = componentsJSON;

	// Process Children
	ordered_json childrenJSON;
	for (int i = 0; i < object.children.size(); i++)
		childrenJSON.push_back(*object.children[i]);
	j["children"] = childrenJSON;
};
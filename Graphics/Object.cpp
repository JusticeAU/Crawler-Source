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
#include "Component.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include <string>

#include "ComponentFactory.h";

using std::to_string;

Object::Object(int objectID, string name)
{
	id = objectID;
	localPosition = { 0,0,0 };
	localRotation = { 0,0,0 };
	localScale = { 1,1,1 };
	transform = mat4(1);
	localTransform = mat4(1);

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
	if (spin) // just some debug spinning for testing lighting.
	{
		localRotation.y += delta * spinSpeed;
		dirtyTransform = true;

		if (localRotation.y > 180) localRotation.y -= 360;
		else if (localRotation.y < -180) localRotation.y += 360;
	}

	if (dirtyTransform) // Our transform has changed. Update it and our childrens transforms.
	{
		localTransform = glm::translate(glm::mat4(1), localPosition)
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.z), glm::vec3{ 0,0,1 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.y), glm::vec3{ 0,1,0 })
			* glm::rotate(glm::mat4(1), glm::radians(localRotation.x), glm::vec3{ 1,0,0 })
			* glm::scale(glm::mat4(1), localScale);

		// Update world transform based on our parent.
		if (parent)
			transform = parent->transform * localTransform;
		else
			transform = localTransform;

		for (auto c : children)
			c->dirtyTransform = true; // set dirty flag on children as they likely need to update now.

	}

	// Update all components
	for (auto component : components)
		component->Update(delta);

	// Update all children recursively.
	for (auto c : children)
		c->Update(delta);

	dirtyTransform = false; // clear our dirty flag
}

void Object::Draw(mat4 pv, vec3 position)
{
	for (auto component : components)
		component->Draw(pv, position);

	for (auto c : children)
		c->Draw(pv, position);
}

// Draws all Imgui data for an object in the scene window.
void Object::DrawGUI()
{
	ImGui::PushID(id);
	if (ImGui::CollapsingHeader(objectName.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
	{
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
			markedForDeletion = true;

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

			ImGui::Checkbox("Rotate", &spin);
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("Speed", &spinSpeed, 1.0f, -50, 50, "%0.1f");
			ImGui::PopItemWidth();
			ImGui::Unindent();
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
	ImGui::PopID();
}

// Used to display the bone hierarchy (which is made up of objects repurposed) without dispalying a bunch of other noisy data that's not used in this context.
void Object::DrawGUISimple()
{
	string idStr = to_string(id);
	string objectStr = objectName + "##" + idStr;
	if (ImGui::TreeNode(objectStr.c_str()))
	{
		if (ImGui::CollapsingHeader("Transform"))
		{
			string positionStr = "Pos##" + to_string(id);
			ImGui::DragFloat3(positionStr.c_str(), &localPosition[0]);

			string rotationStr = "Rot##" + to_string(id);
			ImGui::SliderFloat3(rotationStr.c_str(), &localRotation[0], -180, 180);

			string scaleStr = "Scale##" + to_string(id);
			ImGui::DragFloat3(scaleStr.c_str(), &localScale[0]);

			//ImGui::BeginDisabled();
			/*ImGui::InputFloat4("X", &transform[0].x);
			ImGui::InputFloat4("Y", &transform[1].x);
			ImGui::InputFloat4("Z", &transform[2].x);
			ImGui::InputFloat4("T", &transform[3].x);
			ImGui::InputFloat4("lX", &localTransform[0].x);
			ImGui::InputFloat4("lY", &localTransform[1].x);
			ImGui::InputFloat4("lZ", &localTransform[2].x);
			ImGui::InputFloat4("lT", &localTransform[3].x);*/
			//ImGui::EndDisabled();
		}

		int childCount = (int)children.size();
		string childrenString = "Children (" + to_string(childCount) + ")##" + to_string(id);
		if (ImGui::CollapsingHeader(childrenString.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			for (auto c : children)
				c->DrawGUISimple();
		}
		ImGui::TreePop();
	}
}

void Object::AddChild(Object* child)
{
	child->parent = this;
	children.push_back(child);
}

void Object::CleanUpChildren()
{
	for (int i = 0; i < children.size(); i++)
	{
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

void Object::Write(std::ostream& out)
{
	FileUtils::WriteString(out, objectName);
	
	FileUtils::WriteVec(out, localPosition);
	FileUtils::WriteVec(out, localRotation);
	FileUtils::WriteVec(out, localScale);

	FileUtils::WriteBool(out, spin);
	FileUtils::WriteFloat(out, spinSpeed);

	FileUtils::WriteInt(out, components.size());
	for (int i = 0; i < components.size(); i++)
	{
		FileUtils::WriteUInt(out, components[i]->GetType());
		components[i]->Write(out);
	}

	// write children
	int numChildren = (int)children.size();
	FileUtils::WriteInt(out, numChildren);
	for (int i = 0; i < numChildren; i++)
		children[i]->Write(out);
}

void Object::Read(std::istream& in)
{
	FileUtils::ReadString(in, objectName);
	
	FileUtils::ReadVec(in, localPosition);
	FileUtils::ReadVec(in, localRotation);
	FileUtils::ReadVec(in, localScale);

	FileUtils::ReadBool(in, spin);
	FileUtils::ReadFloat(in, spinSpeed);

	// Components
	unsigned int componentCount;
	FileUtils::ReadUInt(in, componentCount);
	for (int i = 0; i < componentCount; i++)
	{
		unsigned int type;
		FileUtils::ReadUInt(in, type);
		components.push_back(ComponentFactory::ReadComponent(this, in, (ComponentType)type));
	}
	// Have all components run their OnChange function to init/refresh data
	for (auto component : components)
		component->OnParentChange();

	// read children
	int numChildren;
	FileUtils::ReadInt(in, numChildren);
	for (int i = 0; i < numChildren; i++)
	{
		auto o = Scene::CreateObject(this);
		o->Read(in);
	}
}

Component* Object::GetComponent(ComponentType type)
{
	for (auto component : components)
	{
		if (component->GetType() == type)
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

#include "DungeonDecoration.h"
#include "Object.h"
#include "Scene.h"
#include "ComponentRenderer.h"

Crawl::DungeonDecoration::~DungeonDecoration()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonDecoration::UpdateTransform()
{
	object->children[0]->SetLocalPosition(localPosition);
	object->children[0]->SetLocalRotation(localRotation);

}

void Crawl::DungeonDecoration::LoadDecoration()
{
	if (!object)
		return;

	if (modelName == "")
		return;

	if (object->children[0]->children.size() > 0)
		object->children[0]->children[0]->markedForDeletion = true;


	Object* model = Scene::CreateObject(object->children[0]);
	model->LoadFromJSON(ReadJSONFromDisk(modelName));
	UpdateTransform();
	
	renderer = (ComponentRenderer*)object->children[0]->children[0]->GetComponent(Component_Renderer);
}

void Crawl::DungeonDecoration::UpdateShadowCasting()
{
	renderer->castsShadows = castsShadows;
	Scene::s_instance->SetStaticObjectsDirty();
}

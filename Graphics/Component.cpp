#include "Component.h"
#include "Object.h"

void Component::AnnounceChange()
{
	for (auto component : componentParent->components)
		component->OnParentChange();
}

#include "Component.h"
#include "Object.h"

void Component::AnnounceChange()
{
	componentParent->RefreshComponents();
}

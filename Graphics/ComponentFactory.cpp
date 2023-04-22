#include "ComponentFactory.h"

void ComponentFactory::Init()
{
    components.push_back("Model");
    components.push_back("Animator");
    components.push_back("Mesh Renderer");
    components.push_back("Skinned Mesh Renderer");
    components.push_back("Material (Not Yet Implemented)");
    components.push_back("Point Light (Not Yet Implemented)");
    components.push_back("Camera");
    components.push_back("FPSTest");


}

Component* ComponentFactory::NewComponent(Object* parent, int componentIndex)
{
    switch (componentIndex)
    {
    case 0:
        return new ComponentModel(parent);
    case 1:
        return new ComponentAnimator(parent);
    case 2:
        return new ComponentRenderer(parent);
    case 3:
        return new ComponentSkinnedRenderer(parent);
    case 4:
        return nullptr;
    case 5:
        return nullptr;
    case 6:
        return new ComponentCamera(parent);
    case 7:
        return new ComponentFPSTest(parent);

    }
}

Component* ComponentFactory::ReadComponent(Object* parent, std::istream& istream, ComponentType type)
{
    switch (type)
    {
    case Component_Model:
        return new ComponentModel(parent, istream);
    case Component_Animator:
        return new ComponentAnimator(parent, istream);
    case Component_Renderer:
        return new ComponentRenderer(parent, istream);
    case Component_SkinnedRenderer:
        return new ComponentSkinnedRenderer(parent, istream);
    case Component_Material:
        return nullptr;
    case Component_Light:
        return nullptr;
    case Component_Camera:
        return new ComponentCamera(parent, istream);

    }
}

vector<string> ComponentFactory::components;
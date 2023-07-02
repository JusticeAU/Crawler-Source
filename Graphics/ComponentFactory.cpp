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
    components.push_back("AnimationBlender");
    components.push_back("Audio Source");
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
    case 8:
        return new ComponentAnimationBlender(parent);
    case 9:
        return new ComponentAudioSource(parent);

    }

    return nullptr; // shouldn't get here
}

Component* ComponentFactory::ReadComponentJSON(Object* parent, nlohmann::ordered_json j)
{
    string type = j["type"];
    if (type == "Model")
    {
        return new ComponentModel(parent, j);
    }
    else if (type == "Renderer")
    {
        return new ComponentRenderer(parent, j);
    }
    else if (type == "SkinnedRenderer")
    {
        return new ComponentSkinnedRenderer(parent, j);
    }
    else if (type == "Animator")
    {
        return new ComponentAnimator(parent, j);
    }
    else if (type == "Camera")
    {
        return new ComponentCamera(parent, j);
    }

    return nullptr; // shouldn't get here
}

vector<string> ComponentFactory::components;
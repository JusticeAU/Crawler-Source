#include "ComponentFactory.h"
#include "PostProcess.h"

void ComponentFactory::Init()
{
    components.push_back("Model");
    components.push_back("Animator");
    components.push_back("Model Renderer");
    components.push_back("Camera");
    components.push_back("Audio Source");
    components.push_back("Point Light");
    components.push_back("Directional Light (DNU)");
    components.push_back("Billboard");
    components.push_back("Particle System");
}

Component* ComponentFactory::NewComponent(Object* parent, ComponentType type)
{
    switch (type)
    {
    case Component_Model:
        return new ComponentModel(parent);
    case Component_Animator:
        return new ComponentAnimator(parent);
    case Component_Renderer:
        return new ComponentRenderer(parent);
    case Component_Camera:
        return new ComponentCamera(parent);
    case Component_AudioSource:
        return new ComponentAudioSource(parent);
    case Component_LightPoint:
        return new ComponentLightPoint(parent);
    case Component_Billboard:
        return new ComponentBillboard(parent);
    case Component_ParticleSystem:
        return new ComponentParticleSystem(parent);
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
        return new ComponentRenderer(parent, j); // Skinned Rendering functionality was moved in to the standard renderer
    }
    else if (type == "Animator")
    {
        return new ComponentAnimator(parent, j);
    }
    else if (type == "Camera")
    {
        return new ComponentCamera(parent, j);
    }
    else if (type == "Point Light")
    {
        return new ComponentLightPoint(parent, j);
    }

    return nullptr; // shouldn't get here
}

vector<string> ComponentFactory::components;
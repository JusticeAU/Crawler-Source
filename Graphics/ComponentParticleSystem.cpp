#include "ComponentParticleSystem.h"
#include "Object.h"
#include "Graphics.h"
#include "SceneRenderer.h"
#include "MathUtils.h"

#include "VolumeParticleSystem.h"
#include "EmitterParticleSystem.h"
#include "serialisation.h"

ComponentParticleSystem::ComponentParticleSystem(Object* parent) : Component("Particle System", Component_ParticleSystem, parent)
{
    if (!ParticleSystem::isSharedResourcesInitialised)
        ParticleSystem::InitialiseSharedResources();


    /*particleSystem = new VolumeParticleSystem();
    particleSystem->GetShaders();
    particleSystem->Initialise();
    particleSystem->transform = parent->transform;*/
}

ComponentParticleSystem::~ComponentParticleSystem()
{
    delete particleSystem;
}

void ComponentParticleSystem::Update(float delta)
{
    if (particleSystem)
    {
        particleSystem->Update(delta);
        if(componentParent->wasDirtyTransform)
        {
            particleSystem->transform = componentParent->transform;
        }
    }
}

void ComponentParticleSystem::Draw(mat4 pv, vec3 position, DrawMode mode)
{
    if (mode == DrawMode::ParticleSystem)
    {
        if(IsInitialised() && particleSystem->isEnabled) SceneRenderer::AddParticleDraw(this);
    }
}

void ComponentParticleSystem::DrawGUI()
{
    ImGui::SetNextItemWidth(185);
    ImGui::InputText("Name", &name);

	ImGui::SameLine(275);
    ImGui::SetNextItemWidth(100);
    if (ImGui::Button("New")) ImGui::OpenPopup("New Particle System");

    if(ImGui::BeginPopup("New Particle System"))
    {
        ImGui::SeparatorText("System Type");
        if(ImGui::Selectable("Wind Volume"))
        {
            delete particleSystem;
            particleSystem = new VolumeParticleSystem();
            name = "newWindVolume";

        }
        if (ImGui::Selectable("Emitter"))
        {
            delete particleSystem;
            particleSystem = new EmitterParticleSystem();
            name = "newEmitter";
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
	if (!IsConfigured()) ImGui::BeginDisabled();
    if (ImGui::Button("Save")) SaveToDisk(name);
    if (!IsConfigured()) ImGui::EndDisabled();

    ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
    if (ImGui::Button("Load")) ImGui::OpenPopup("Load Particle System");

    if (ImGui::BeginPopup("Load Particle System"))
    {
        //ImGui::SameLine();
        ImGui::SeparatorText("Particle Configurations");
        for (auto d : fs::recursive_directory_iterator(path))
        {
            if (d.path().has_extension() && d.path().extension() == ".particle")
            {
                string foundParticlePath = d.path().relative_path().string();
                std::replace(foundParticlePath.begin(), foundParticlePath.end(), '\\', '/');
                if (ImGui::Selectable(d.path().filename().string().c_str()))
                {
                    LoadFromDisk(foundParticlePath);
                    name = d.path().stem().string();
                    particleSystem->Initialise();
                    particleSystem->SetEnabled(true);
                    particleSystem->Play();
                }
            }
        }
        ImGui::EndPopup();
    }


    if (!IsInitialised()) ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(100);
    if (ImGui::Button(IsPlaying() ? "Pause" : "Play"))  particleSystem->isPlaying = !particleSystem->isPlaying;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::Button(IsEnabled() ? "Disable" : "Enable"))  particleSystem->isEnabled = !particleSystem->isEnabled;
    if (!IsInitialised()) ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (!IsConfigured()) ImGui::BeginDisabled();
    if (ImGui::Button(IsInitialised() ? "Reinitialise" : "Initialise")) particleSystem->Initialise();
    if (!IsConfigured()) ImGui::EndDisabled();
	if(IsConfigured()) particleSystem->DrawGUI();
}

void ComponentParticleSystem::SaveToDisk(const string& filename)
{
    ordered_json j;
    switch(particleSystem->type)
    {
    case ParticleSystem::Type::Volume:
	{
        j = *(VolumeParticleSystem*)particleSystem;
        break;
	}
    case ParticleSystem::Type::Emitter:
    {
        j = *(EmitterParticleSystem*)particleSystem;
        break;
    }
    }
    WriteJSONToDisk(path + filename + ".particle", j);
}

void ComponentParticleSystem::LoadFromDisk(const string& filename)
{
    delete particleSystem;

    ordered_json j = ReadJSONFromDisk(filename);
    ParticleSystem::Type type = j["type"];
    switch(type)
    {
    case ParticleSystem::Type::Volume:
    {
        particleSystem = new VolumeParticleSystem();
        j.get_to(*(VolumeParticleSystem*)particleSystem);
        break;
    }
    case ParticleSystem::Type::Emitter:
    {
        particleSystem = new EmitterParticleSystem();
        j.get_to(*(EmitterParticleSystem*)particleSystem);
        break;
    }
    }
    particleSystem->transform = componentParent->transform;
}

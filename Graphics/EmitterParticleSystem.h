#pragma once
#include "BaseParticleSystem.h"

class ShaderProgram;
class Texture;

class EmitterParticleSystem : public ParticleSystem
{
public:
    EmitterParticleSystem();

    void UpdateParticles(float delta) override;
    void Draw() override;
    void DrawGUI() override;

    void SetShaderUniforms() override;
    void BindDrawTextures() override;
    void UnBindDrawTextures() override;

    void Initialise() override;

    void InitialiseTransformFeedbackVaryings(GLuint shaderProgramID) override;

    unsigned int maximumParticles = 500;
    std::string textureString = "crawler/texture/particle/particle.tga";
    float size = 0.20f;

    float time = 0.0f;

    float particlesSec = 60.0f;

    glm::vec2 particleTTLRange = { 1, 2 };


    glm::vec3 convergencePoint = { 0,0,1 };
    float convergenceStrength = 1.0f;
    glm::vec3 windDirection = { 0.,0.,5. };

private:
    // Internal Data
    struct Particle
    {
        glm::vec3 position;

        float type;
        glm::vec3 velocity;

        float age = 0.0f;
        float TTL = 1.0f;
    };

    Texture* texture = nullptr;

    bool drawGizmos = false;
    glm::vec3 gizmoColour = glm::vec3(1, 1, 1);
};

inline void to_json(nlohmann::ordered_json& j, const EmitterParticleSystem& obj)
{
    j["type"] = obj.type;

    j["maximumParticles"] = obj.maximumParticles;
    j["texture"] = obj.textureString;

    j["size"] = obj.size;

    j["particlesSec"] = obj.particlesSec;

    j["particleTTLRange"] = obj.particleTTLRange;


    j["convergencePoint"] = obj.convergencePoint;
	j["convergenceStrength"] = obj.convergenceStrength;

	j["windDirection"] = obj.windDirection;
}

inline void from_json(const nlohmann::ordered_json& j, EmitterParticleSystem& obj)
{
    j.at("texture").get_to(obj.textureString);

    j.at("maximumParticles").get_to(obj.maximumParticles);
    j.at("size").get_to(obj.size);

    j.at("particlesSec").get_to(obj.particlesSec);

    j.at("particleTTLRange").get_to(obj.particleTTLRange);
    
    
    j.at("convergencePoint").get_to(obj.convergencePoint);
    j.at("convergenceStrength").get_to(obj.convergenceStrength);

    j.at("windDirection").get_to(obj.windDirection);

}
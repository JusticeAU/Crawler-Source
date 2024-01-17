#pragma once
#include "BaseParticleSystem.h"

class ShaderProgram;
class Texture;

class VolumeParticleSystem : public ParticleSystem
{
public:
    VolumeParticleSystem();

	void UpdateParticles(float delta) override;
	void Draw() override;
    void DrawGUI() override;

    void SetVertexAttribPointers() const override;
    void ClearVertexAttribPointers() const override;

    void SetShaderUniforms() override;
    void BindDrawTextures() override;
    void UnBindDrawTextures() override;

	void Initialise() override;

    void InitialiseTransformFeedbackVaryings(GLuint shaderProgramID) override;

    float billboardRotation = 0.0f;

    // Serialised Data - Save/Load
    unsigned int particleCount = 1000;
	std::string textureString = "crawler/texture/particle/particle.tga";
    float size = 0.20f;

    glm::vec3 minBounds = { -10,-10, 0 };
    glm::vec3 maxBounds = { 10,10, 10 };

    float time = 0.0f;

    glm::vec3 windDirection = { -1.5,-1.,-10 };

    float randomVelocityScale = 20.0f;
    float randomVelocityFrequency = 0.2f;
    float maxVelocity = 1.0f;

    float minStartRotation = 0;
    float maxStartRotation = 0;

    float minRotationSpeed = 0;
    float maxRotationSpeed = 0;


private:
    // Internal Data
    struct Particle
    {
        glm::vec3 position = glm::vec3(0);
        glm::vec3 velocity = glm::vec3(0);;
        float rotationDegrees = 0;
        float rotationSpeed = 0;
    };

    Texture* texture = nullptr;
    bool m_debugDrawBounds = false;
};

inline void to_json(nlohmann::ordered_json& j, const VolumeParticleSystem& obj)
{
    j["type"] = obj.type;

    j["count"] = obj.particleCount;
    j["texture"] = obj.textureString;

    j["size"] = obj.size;

    j["minBounds"] = obj.minBounds;
    j["maxBounds"] = obj.maxBounds;

    j["windDirection"] = obj.windDirection;

    j["randomVelocityScale"] = obj.randomVelocityScale;
    j["randomVelocityFrequency"] = obj.randomVelocityFrequency;
    j["maxVelocity"] = obj.maxVelocity;

    j["minStartRotation"] = obj.minStartRotation;
    j["maxStartRotation"] = obj.maxStartRotation;
    j["minRotationSpeed"] = obj.minRotationSpeed;
    j["maxRotationSpeed"] = obj.maxRotationSpeed;
}

inline void from_json(const nlohmann::ordered_json& j, VolumeParticleSystem& obj)
{
	j.at("texture").get_to(obj.textureString);

    j.at("count").get_to(obj.particleCount);
    j.at("size").get_to(obj.size);

    j.at("minBounds").get_to(obj.minBounds);
    j.at("maxBounds").get_to(obj.maxBounds);

    j.at("windDirection").get_to(obj.windDirection);

    j.at("randomVelocityScale").get_to(obj.randomVelocityScale);
    j.at("randomVelocityFrequency").get_to(obj.randomVelocityFrequency);
    j.at("maxVelocity").get_to(obj.maxVelocity);

    j.at("minStartRotation").get_to(obj.minStartRotation);
    j.at("maxStartRotation").get_to(obj.maxStartRotation);
    j.at("minRotationSpeed").get_to(obj.minRotationSpeed);
    j.at("maxRotationSpeed").get_to(obj.maxRotationSpeed);
}
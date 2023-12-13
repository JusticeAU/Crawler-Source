#include "VolumeParticleSystem.h"
#include "MathUtils.h"
#include "TextureManager.h"
#include "ShaderProgram.h"
#include <iostream>
#include "LineRenderer.h"

using glm::vec4;
using glm::vec3;
using glm::mat4;

VolumeParticleSystem::VolumeParticleSystem() : ParticleSystem()
{
    type = Volume;

    updateShaderString = "engine/shader/particle/WindVolumeUpdate";
    drawShaderString = "engine/shader/particle/WindVolumeDraw";

    GetShaders();
    
    // used by the renderer!
    particleStructSize = sizeof(Particle);
    particleStructPositionOffset = offsetof(Particle, position);

    texture = TextureManager::GetTexture(textureString);
}

void VolumeParticleSystem::UpdateParticles(float delta)
{
    delta *= timeScale;

    time += delta;
    if (time > 1000.0f) time -= 1000.0f;

    glEnable(GL_RASTERIZER_DISCARD);
    // Update
    updateShader->Bind();
    updateShader->SetFloatUniform("gDeltaTimeMillis", delta);
    updateShader->SetFloatUniform("gTime", time);

    randomVec3s1D->Bind(1);
    updateShader->SetIntUniform("gRandomTexture", 1);
    updateShader->SetVector3Uniform("gBoundsMin", minBounds/* + Scene::s_instance->GetCurrentCamera()->GetWorldSpacePosition()*/);
    updateShader->SetVector3Uniform("gBoundsMax", maxBounds/* + Scene::s_instance->GetCurrentCamera()->GetWorldSpacePosition()*/);
    updateShader->SetVector3Uniform("gWindDirection", windDirection);
    updateShader->SetFloatUniform("gRandomVelocityScale", randomVelocityScale);
    updateShader->SetFloatUniform("gRandomVelocityFrequency", randomVelocityFrequency);

    updateShader->SetFloatUniform("gMaxParticleVelocity", maxVelocity);

    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currVB]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[currTFB]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, position)); // position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, velocity));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, rotationDegrees));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, rotationSpeed));




    //GLuint tfQuery = -1;
    //glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, tfQuery);


    glBeginTransformFeedback(GL_POINTS);

    if (isFirst) {
        glDrawArrays(GL_POINTS, 0, particleCount);
        isFirst = false;
    }
    else
    {
        glDrawTransformFeedback(GL_POINTS, transformFeedback[currVB]);
    }

    glEndTransformFeedback();
    //glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    /*GLuint numPrimitivesWritten = 0;
    glGetQueryObjectuiv(tfQuery, GL_QUERY_RESULT, &numPrimitivesWritten);
    std::cout << particleBuffer[currVB] << std::endl;
    std::cout << transformFeedback[currTFB] << std::endl;
    std::cout << numPrimitivesWritten << std::endl;

    std::cout << "---------\n";*/
}

void VolumeParticleSystem::Draw()
{
}

void VolumeParticleSystem::DrawGUI()
{
    ImGui::Text("Wind Volume Particle System");
    ImGui::SeparatorText("Particle");
    TextureManager::DrawGUITextureSelector("Texture", &texture, &textureString);
    ImGui::SetNextItemWidth(150);
    int particleCountUI = particleCount;
    ImGui::InputInt("Particle Count", &particleCountUI, 1, 10);
    if(ImGui::IsItemDeactivatedAfterEdit())
    {
        particleCount = glm::clamp(particleCountUI, 0, INT_MAX);
        Initialise();
    }
    ImGui::SetNextItemWidth(50);
    ImGui::DragFloat("Particle Scale", &size, 0.05, 0, 10);
    ImGui::SetNextItemWidth(50);
    ImGui::DragFloat("Max Velocity", &maxVelocity, 0.1f, 0, 100);

    ImGui::SeparatorText("Wind");
	ImGui::SetNextItemWidth(200);
    ImGui::DragFloat3("Velocity", &windDirection.x, 0.1f, -100, 100);
    ImGui::SetNextItemWidth(50);
    ImGui::DragFloat("Noise Scale", &randomVelocityScale, 0.1f, 0, 10000);
    ImGui::SetNextItemWidth(50);
    ImGui::DragFloat("Noise Frequency", &randomVelocityFrequency, 0.1f, 0, 10000);

    ImGui::SeparatorText("Rotation");
    ImGui::PushID("Start");
    ImGui::SetNextItemWidth(50);
    if (ImGui::DragFloat("Min", &minStartRotation, 1.00, -180, maxStartRotation)) Initialise();
    ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
    if (ImGui::DragFloat("Max Start Rotation", &maxStartRotation, 1.00, minStartRotation, 180)) Initialise();
    ImGui::PopID();
    ImGui::PushID("Speed");
	ImGui::SetNextItemWidth(50);
    if (ImGui::DragFloat("Min", &minRotationSpeed, 1.00, 0, maxRotationSpeed)) Initialise();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    if (ImGui::DragFloat("Max Rotation Speed", &maxRotationSpeed, 1.00, minRotationSpeed, 9999)) Initialise();
    ImGui::PopID();


    ImGui::SeparatorText("Volume Bounds");
	ImGui::SetNextItemWidth(200);
    ImGui::DragFloat3("Min", &minBounds.x);
    ImGui::SetNextItemWidth(200);
    ImGui::DragFloat3("Max ", &maxBounds.x);
    ImGui::Checkbox("Show Bounds", &m_debugDrawBounds);
    ImGui::SliderFloat("Time Scale", &timeScale, -1, 1);

	if (m_debugDrawBounds)
    {
        //constexpr mat4 transform(1);

        glm::vec3 points[8];
        points[0] = minBounds;
        points[1] = { maxBounds.x, minBounds.y, minBounds.z };
        points[2] = { maxBounds.x, maxBounds.y, minBounds.z };
        points[3] = { minBounds.x, maxBounds.y, minBounds.z };
        points[4] = { minBounds.x, minBounds.y, maxBounds.z };
        points[5] = { maxBounds.x, minBounds.y, maxBounds.z };
        points[6] = maxBounds;
        points[7] = { minBounds.x, maxBounds.y, maxBounds.z };


        points[0] = vec3(transform * vec4(points[0], 1));
        points[1] = vec3(transform * vec4(points[1], 1));
        points[2] = vec3(transform * vec4(points[2], 1));
        points[3] = vec3(transform * vec4(points[3], 1));
        points[4] = vec3(transform * vec4(points[4], 1));
        points[5] = vec3(transform * vec4(points[5], 1));
        points[6] = vec3(transform * vec4(points[6], 1));
        points[7] = vec3(transform * vec4(points[7], 1));

        LineRenderer::DrawBoxFromPoints(&points[0]);
    }
}

void VolumeParticleSystem::SetVertexAttribPointers() const
{
    ParticleSystem::SetVertexAttribPointers();
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, particleStructSize, (const void*)offsetof(Particle, rotationDegrees));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, particleStructSize, (const void*)offsetof(Particle, rotationSpeed));

}

void VolumeParticleSystem::ClearVertexAttribPointers() const
{
	ParticleSystem::ClearVertexAttribPointers();
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void VolumeParticleSystem::SetShaderUniforms()
{
    drawShader->SetFloatUniform("quadScale", size);
    drawShader->SetIntUniform("gColorMap", 1);
}

void VolumeParticleSystem::BindDrawTextures()
{
    texture->Bind(1);
}

void VolumeParticleSystem::UnBindDrawTextures()
{
    texture->Bind(0);
}

void VolumeParticleSystem::Initialise()
{
    if (isInitialised)
    {
        DeleteBuffers();
        isFirst = true;
        currVB = 0;
        currTFB = 1;
    }

    texture = TextureManager::GetTexture(textureString);
    Particle* initialParticles = new Particle[particleCount];

    for (unsigned int i = 0; i < particleCount; i++)
    {
        initialParticles[i].position.x = MathUtils::Lerp(minBounds.x, maxBounds.x, (float)rand() / RAND_MAX);
        initialParticles[i].position.y = MathUtils::Lerp(minBounds.y, maxBounds.y, (float)rand() / RAND_MAX);
        initialParticles[i].position.z = MathUtils::Lerp(minBounds.z, maxBounds.z, (float)rand() / RAND_MAX);
        initialParticles[i].velocity = windDirection;
        initialParticles[i].rotationDegrees = MathUtils::Lerp(minStartRotation, maxStartRotation, (float)rand() / RAND_MAX);
        initialParticles[i].rotationSpeed = MathUtils::Lerp(minRotationSpeed, maxRotationSpeed, (float)rand() / RAND_MAX);
        // and just randomly flip invert the speed
        if (rand() > RAND_MAX * 0.5f) initialParticles[i].rotationSpeed = -initialParticles[i].rotationSpeed;
    }

    glGenTransformFeedbacks(2, transformFeedback);
    glGenBuffers(2, particleBuffer);

    // Create transform feedback and standard buffers (2x each)
    for (unsigned int i = 0; i < 2; i++) {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[i]);
        glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[i]);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * particleCount, i == 0 ? initialParticles : NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]);
    }
    delete[] initialParticles;

    isInitialised = true;
}

void VolumeParticleSystem::InitialiseTransformFeedbackVaryings(GLuint shaderProgramID)
{
    const GLchar* Varyings[4];
    Varyings[0] = "Position1";
    Varyings[1] = "Velocity1";
    Varyings[2] = "RotationDegrees1";
    Varyings[3] = "RotationSpeed1";
    glTransformFeedbackVaryings(shaderProgramID, 4, Varyings, GL_INTERLEAVED_ATTRIBS);
}

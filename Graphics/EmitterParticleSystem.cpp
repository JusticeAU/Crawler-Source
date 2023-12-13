#include "EmitterParticleSystem.h"

#include "LineRenderer.h"
#include "MathUtils.h"
#include "TextureManager.h"
#include "ShaderProgram.h"

EmitterParticleSystem::EmitterParticleSystem()
{
    type = Emitter;

    updateShaderString = "engine/shader/particle/Emitter";
    GetShaders();

    // used by the renderer!
    particleStructSize = sizeof(Particle);
    particleStructPositionOffset = offsetof(Particle, position);

    texture = TextureManager::GetTexture(textureString);
}

void EmitterParticleSystem::UpdateParticles(float delta)
{
    delta *= timeScale;

    time += delta;
    if (time > 1.0f) time -= 1.0f;

    glEnable(GL_RASTERIZER_DISCARD);
    // Update
    updateShader->Bind();
    updateShader->SetFloatUniform("gDeltaTimeMillis", delta);
    updateShader->SetFloatUniform("gTime", time);

    updateShader->SetFloatUniform("gEmitTime", 1 / particlesSec);
    updateShader->SetFloatUniform("gParticleTTLMin", particleTTLRange.x);
    updateShader->SetFloatUniform("gParticleTTLMax", particleTTLRange.y);

    updateShader->SetVector3Uniform("gConvergencePoint", convergencePoint);
    updateShader->SetFloatUniform("gConvergenceStrength", convergenceStrength);




    randomVec3s1D->Bind(1);
    updateShader->SetIntUniform("gRandomTexture", 1);
    //updateShader->SetVector3Uniform("gBoundsMin", minBounds/* + Scene::s_instance->GetCurrentCamera()->GetWorldSpacePosition()*/);
    //updateShader->SetVector3Uniform("gBoundsMax", maxBounds/* + Scene::s_instance->GetCurrentCamera()->GetWorldSpacePosition()*/);
    updateShader->SetVector3Uniform("gWindDirection", windDirection);
    //updateShader->SetFloatUniform("gRandomVelocityScale", randomVelocityScale);
    //updateShader->SetFloatUniform("gRandomVelocityFrequency", randomVelocityFrequency);

    //updateShader->SetFloatUniform("gMaxParticleVelocity", maxVelocity);

    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currVB]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[currTFB]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, position));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, type));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, velocity));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, age));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const void*)offsetof(Particle, TTL));

    //GLuint tfQuery = -1;
    //glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, tfQuery);


    glBeginTransformFeedback(GL_POINTS);

    if (isFirst) {
        glDrawArrays(GL_POINTS, 0, 1);
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
    glDisableVertexAttribArray(4);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    //GLuint numPrimitivesWritten = 0;
    //glGetQueryObjectuiv(tfQuery, GL_QUERY_RESULT, &numPrimitivesWritten);
    //std::cout << particleBuffer[currVB] << std::endl;
    //std::cout << transformFeedback[currTFB] << std::endl;
    //std::cout << numPrimitivesWritten << std::endl;

    //std::cout << "---------\n";
}

void EmitterParticleSystem::Draw()
{
}

void EmitterParticleSystem::DrawGUI()
{
    ImGui::Text("Emitter Particle System");
    ImGui::SeparatorText("Emitter");
    ImGui::SetNextItemWidth(75);
    ImGui::DragFloat("Particles (Per Sec)", &particlesSec, 1, 0.01f, maximumParticles);
	ImGui::SetNextItemWidth(150);
    int maximumParticlesUI = maximumParticles;
    ImGui::InputInt("Maximum Particles", &maximumParticlesUI, 1, 10);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        maximumParticles = glm::clamp(maximumParticlesUI, 0, INT_MAX);
        Initialise();
    }

    ImGui::SeparatorText("Particle");
    TextureManager::DrawGUITextureSelector("Texture", &texture, &textureString);
    ImGui::SetNextItemWidth(150);
	ImGui::DragFloat2("Life (Min/Max Sec)", &particleTTLRange.x, 0.1f, 0.1f, 99);
    ImGui::SetNextItemWidth(75);
    ImGui::DragFloat("Size", &size, 0.05f, 0, 10);

	ImGui::SeparatorText("Converge");
	ImGui::SetNextItemWidth(200);
    ImGui::DragFloat3("Point", &convergencePoint.x,0.01f);
    ImGui::SetNextItemWidth(75);
    ImGui::DragFloat("Strength", &convergenceStrength, 0.1f);


    ImGui::SeparatorText("Wind");
    ImGui::SetNextItemWidth(200);
    ImGui::DragFloat3("Velocity", &windDirection.x, 0.1f, -100, 100);

	ImGui::SeparatorText("Gizmos");
    ImGui::Checkbox("Show", &drawGizmos);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::ColorEdit3("Colour", &gizmoColour.x);
    if(drawGizmos)
    {
        LineRenderer::DrawLine(transform[3], (glm::vec3)transform[3] + glm::vec3(0, 0, 0.1), gizmoColour);

    	LineRenderer::DrawLine(convergencePoint, convergencePoint + glm::vec3(0, 0, 0.1), gizmoColour);
    }
}

void EmitterParticleSystem::SetShaderUniforms()
{
    drawShader->SetFloatUniform("quadScale", size);
    drawShader->SetIntUniform("gColourMap", 1);
}

void EmitterParticleSystem::BindDrawTextures()
{
    texture->Bind(1);
}

void EmitterParticleSystem::UnBindDrawTextures()
{
    texture->Bind(0);
}

void EmitterParticleSystem::Initialise()
{
    if (isInitialised)
    {
        DeleteBuffers();
        isFirst = true;
    }

    texture = TextureManager::GetTexture(textureString);
    Particle* initialParticles = new Particle[maximumParticles];

    // create the emitter particle.
    initialParticles[0].position = { 0,0,0 };
    initialParticles[0].type = 0; // emitter;
    initialParticles[0].velocity = {0,0,0};
    initialParticles[0].age = 0; // emitter;
    initialParticles[0].TTL = 0; // emitter;

    glGenTransformFeedbacks(2, transformFeedback);
    glGenBuffers(2, particleBuffer);

    // Create transform feedback and standard buffers (2x each)
    for (unsigned int i = 0; i < 2; i++) {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[i]);
        glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * maximumParticles, initialParticles, GL_DYNAMIC_DRAW); // only send data for the first particle, its all we'll need.
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]);
    }
    delete[] initialParticles;

    isInitialised = true;
}

void EmitterParticleSystem::InitialiseTransformFeedbackVaryings(GLuint shaderProgramID)
{
    const GLchar* Varyings[5];
    Varyings[0] = "Position1";
    Varyings[1] = "Type1";
    Varyings[2] = "Velocity1";
    Varyings[3] = "Age1";
    Varyings[4] = "TTL1";

    glTransformFeedbackVaryings(shaderProgramID, 5, Varyings, GL_INTERLEAVED_ATTRIBS);
}

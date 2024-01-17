#include "BaseParticleSystem.h"
#include "ShaderManager.h"
#include "ShaderProgram.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "Texture.h"

ParticleSystem::~ParticleSystem()
{
	//delete randomVec3s1D;
	DeleteBuffers();
}

void ParticleSystem::Update(float delta)
{
	if(isFirst && isEnabled)
	{
		UpdateParticles(delta);
		return;
	}

	if (isPlaying && isEnabled)
	{
		SwapBuffers();
		UpdateParticles(delta);
	}
}

void ParticleSystem::InitialiseSharedResources()
{
	randomVec3s1D = new Texture();
	randomVec3s1D->CreateRandomTexture(128);
}

void ParticleSystem::DrawBuffers() const
{
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currTFB]);

	SetVertexAttribPointers(); // virtual function. inherting classes should override this to set their own attributes

	glDrawTransformFeedback(GL_POINTS, transformFeedback[currTFB]);

	ClearVertexAttribPointers();
}

void ParticleSystem::SetVertexAttribPointers() const
{
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, particleStructSize, (const void*)particleStructPositionOffset); // position
}

void ParticleSystem::ClearVertexAttribPointers() const
{
	glDisableVertexAttribArray(0);
}

void ParticleSystem::SwapBuffers()
{
	currVB = currTFB;
	currTFB = (currTFB + 1) & 0x1;
}

void ParticleSystem::DeleteBuffers()
{
	glDeleteTransformFeedbacks(1, &transformFeedback[0]);
	glDeleteTransformFeedbacks(1, &transformFeedback[1]);
	glDeleteBuffers(1, &particleBuffer[0]);
	glDeleteBuffers(1, &particleBuffer[1]);
}

void ParticleSystem::GetShaders()
{
	if (ShaderManager::ShaderProgramExists(updateShaderString))
		updateShader = ShaderManager::GetShaderProgram(updateShaderString);
	else
	{
		// Load shader
		string SHAD = FileUtils::LoadFileAsString(updateShaderString + ".PART");
		int count = 0;
		string* SHADs = StringUtils::Split(SHAD, "\n", &count);
		string subfolder = updateShaderString.substr(0, updateShaderString.find_last_of('/') + 1);

		// Build the shader from the serilised names that are assumed to be paths relative to the current sub folder
		ShaderProgram* shader = new ShaderProgram();
		shader->CreateShaderProgram();
		shader->LoadStageFromFile(subfolder + SHADs[0], GL_GEOMETRY_SHADER);
		shader->LoadStageFromFile(subfolder + SHADs[1], GL_VERTEX_SHADER);
		InitialiseTransformFeedbackVaryings(shader->GetShaderProgramID());
		shader->Link();

		shader->name = updateShaderString;
		ShaderManager::AddShaderProgram(updateShaderString, shader);

		updateShader = shader;

		delete[] SHADs;
	}

	if (ShaderManager::ShaderProgramExists(drawShaderString))
		drawShader = ShaderManager::GetShaderProgram(drawShaderString);
	else
	{
		// Load shader
		string SHAD = FileUtils::LoadFileAsString(drawShaderString + ".SHAD");
		int count = 0;
		string* SHADs = StringUtils::Split(SHAD, "\n", &count);
		string subfolder = drawShaderString.substr(0, drawShaderString.find_last_of('/') + 1);

		// Build the shader from the serilised names that are assumed to be paths relative to the current sub folder
		ShaderProgram* shader = new ShaderProgram();
		shader->CreateShaderProgram();
		shader->LoadStageFromFile(subfolder + SHADs[0], GL_GEOMETRY_SHADER);
		shader->LoadStageFromFile(subfolder + SHADs[1], GL_VERTEX_SHADER);
		shader->LoadStageFromFile(subfolder + SHADs[2], GL_FRAGMENT_SHADER);
		shader->Link();

		shader->name = drawShaderString;
		ShaderManager::AddShaderProgram(drawShaderString, shader);

		drawShader = shader;

		delete[] SHADs;
	}
}

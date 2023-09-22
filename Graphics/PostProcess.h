#pragma once
#include "Graphics.h"
#include "serialisation.h"
#include <string>

class FrameBuffer;
class ShaderProgram;
class Mesh;
class ComponentCamera;

using std::string;

class PostProcess
{
public:
	PostProcess(string name);
	~PostProcess();

	PostProcess(PostProcess& other) = delete;
	const PostProcess& operator=(const PostProcess& other) = delete;

	void BindFrameBuffer();
	void Process(ComponentCamera* camera);
	void BindOutput();

	void SetShader(ShaderProgram* shader) { m_shader = shader; }
	void SetShaderName(string shaderName) { m_shaderName = shaderName; } // for loading later on.
	string GetShaderName() { return m_shaderName; }

	string GetName() { return m_name; }

	static void PassThrough(bool ownShaderBound = false);
	string m_name;
protected:
	string m_shaderName;
	ShaderProgram* m_shader = nullptr;
	FrameBuffer* m_frameBuffer = nullptr;

	static Mesh* s_frame;
	static ShaderProgram* s_passthroughShader;
	static const int m_textureBindPoint = 20; // 20 is a solid number.

	static void Init();
};


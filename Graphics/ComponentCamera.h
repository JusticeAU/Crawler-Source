#pragma once
#include "Component.h"
#include "Graphics.h"
#include "Object.h"
#include <string>
#include <vector>

class Camera;
class FrameBuffer;
class Object;
class PostProcess;

using std::vector;

class ComponentCamera : public Component
{
public:
	ComponentCamera(Object* parent);
	ComponentCamera(Object* parent, std::istream& istream);

	~ComponentCamera();
	ComponentCamera(ComponentCamera& other) = delete;
	const ComponentCamera& operator=(const ComponentCamera& other) = delete;

	void Update(float deltatime) override;

	void DrawGUI() override;

	void Write(std::ostream& ostream) override;

	void RunPostProcess();

	int postProcessDev = 0;

public:
	float nearClip = 0.1f;
	float farClip = 2000.0f;

	float fieldOfView = 45;
	float aspect = 16/(float)9;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 matrix;

	FrameBuffer* frameBuffer;
	FrameBuffer* m_frameBufferProcessed;
	
	bool dirtyConfig = false;

	Object* cameraGizmo;

	// Post processing
	vector<PostProcess*> m_postProcessStack;

};
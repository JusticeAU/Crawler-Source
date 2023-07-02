#pragma once
#include "Component.h"
#include "Graphics.h"
#include "Object.h"
#include "AudioListener.h"
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
	ComponentCamera(Object* parent, ordered_json j);

	~ComponentCamera();
	ComponentCamera(ComponentCamera& other) = delete;
	const ComponentCamera& operator=(const ComponentCamera& other) = delete;

	void Update(float deltatime) override;

	void DrawGUI() override;

	void UpdateViewProjectionMatrix();

	const vec3 GetWorldSpacePosition() { return componentParent->GetWorldSpacePosition(); }
	const mat4 GetViewProjectionMatrix() { return matrix; }

	AudioListener* GetAudioListener() { return &m_audioListener; }

	void SetAsRenderTarget();
	// Runs the post processing stack. This is also required to run to transfer the frame from the Raw to Processed Framebuffer, regardless of if there is a stack or not.
	void RunPostProcess();

	float nearClip = 0.1f;
	float farClip = 2000.0f;

	float fieldOfView = 45;
	float aspect = 16/(float)9;
protected:

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 matrix;

	AudioListener m_audioListener;
	void UpdateAudioListener();

	// Store framebuffers for pre and post processing, we can access them as textures if we want, for debugging or whatever.
	FrameBuffer* m_frameBufferRaw;
	FrameBuffer* m_frameBufferProcessed;
	
	// Flag for if the matrix should be rebuild. Can be triggered by UI interaction. (parent object dirty transform is also checked)
	bool dirtyConfig = true;

	// dummy object to store info about rendering the Camera Gizmo. This should probably move in to some other UI handler or something. This component doesnt really need to be aware of its GUI context.
	Object* cameraGizmo;

	// Stores our postprocess containers. These camera renders its scene to the Raw framebuffer, then iterates over this list of post processing effects. Then renders to processed.
public:
	vector<PostProcess*> m_postProcessStack;
};
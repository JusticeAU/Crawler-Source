#pragma once
#include "Component.h"
#include "Graphics.h"
#include "Object.h"
#include "AudioListener.h"
#include "Primitives.h"
#include <string>
#include <vector>

class Object;
class FrameBuffer;
class PostProcess;

using std::vector;

class ComponentCamera : public Component
{
public:
	ComponentCamera(Object* parent, bool noGizmo = false); // No Gizmo is a bit of a hack for my crude gizmo imlpementation. The Scene Editor camera doesnt need a gizmo.
	ComponentCamera(Object* parent, ordered_json j);

	~ComponentCamera();
	ComponentCamera(ComponentCamera& other) = delete;
	const ComponentCamera& operator=(const ComponentCamera& other) = delete;

	void Update(float deltatime) override;

	void DrawGUI() override;

	void UpdateViewProjectionMatrix();

	const vec3 GetWorldSpacePosition() { return componentParent->GetWorldSpacePosition(); }
	const mat4 GetViewProjectionMatrix() { return matrix; }
	const mat4 GetProjectionMatrix() { return projection; }
	const mat4 GetViewMatrix() { return view; }

	void UpdateFrustum();
	const vec3 GetRayFromNDC(glm::vec2 NDC); // Returns a Vector pointing in to the scene from the camera near plane, from NDC.

	void SetAspect(float aspect);
	AudioListener* GetAudioListener() { return &m_audioListener; }

	// Runs the post processing stack. This is also required to run to transfer the frame from the Raw to Processed Framebuffer, regardless of if there is a stack or not.
	void RunPostProcess(FrameBuffer* outBuffer);

	float nearClip = 0.1f;
	float farClip = 2000.0f;
	CameraFrustum frustum;

	float fieldOfView = 45;
	float aspect = 16/(float)9;
protected:
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 matrix;


	AudioListener m_audioListener;
	void UpdateAudioListener();
	
	// Flag for if the matrix should be rebuild. Can be triggered by UI interaction. (parent object dirty transform is also checked)
	bool dirtyConfig = true;

	// dummy object to store info about rendering the Camera Gizmo. This should probably move in to some other UI handler or something. This component doesnt really need to be aware of its GUI context.
	Object* cameraGizmo;
public:
	// Stores our postprocess containers. These camera renders its scene to the Raw framebuffer, then iterates over this list of post processing effects. Then renders to processed.
	vector<PostProcess*> m_postProcessStack;
};
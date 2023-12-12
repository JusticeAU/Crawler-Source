#pragma once

class Object;
class ComponentCamera;

// Wrapper for a camera componen used for scene editing purposes. It shoudn't be a member of the scene, so it's contained here.
class SceneEditorCamera
{
public:
	SceneEditorCamera();

	void Update(float deltaTime);

	void DrawGUI();

	Object* object;
	ComponentCamera* camera;

	bool isAdjusting = false;
	float moveSpeed = 15.0f;
	float lookSpeed = 0.1;
};
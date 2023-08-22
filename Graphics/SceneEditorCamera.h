#pragma once

class Object;
class ComponentCamera;

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
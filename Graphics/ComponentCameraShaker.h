#pragma once

class Object;

class ComponentCameraShaker
{
public:
	ComponentCameraShaker();
	void Update(float delta);
	Object* cameraObject = nullptr;

	float xTranslation, zTranslation, yTranslation;
	float xTranslationFrequency, yTranslationFrequency, zTranslationFrequenc;
	float xRotation, zRotation, yRotation;
	float xRotationFrequency, yRotationFrequency, zRotationFrequency;


};
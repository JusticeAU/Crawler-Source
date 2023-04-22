#include "ComponentFPSTest.h"
#include "ComponentAnimator.h"
#include "Object.h"
#include "Input.h"


void ComponentFPSTest::Update(float delta)
{
	// Testing what can be done with existing configuration
	// Ideally should have an input manager type setup. Components shouldnt talk to glfw.
	// an animation state machine would be cool too. it should talk to the animator component, or perhaps contain it.

	if (nextAnimation != "")
	{
		nextAnimTime -= delta;

		if (nextAnimTime <= 0.0f)
		{
			animator->StartAnimation(nextAnimation, nextLooping);
			nextAnimation = "";
		}
	}
	if (glfwGetMouseButton(window, 0) && !fireDown)
	{
		fireDown = true;
		clipRounds -= 1;
		animator->StartAnimation("Armature|FPS_Pistol_Fire");
		nextAnimation = "Armature|FPS_Pistol_Idle";
		nextLooping = true;
		nextAnimTime = 0.3f;
	}
	if(!glfwGetMouseButton(window, 0))
		fireDown = false;
	if (glfwGetKey(window, GLFW_KEY_R) && !reloadDown)
	{
		reloadDown = true;
		clipRounds = clipCapacity;
		animator->StartAnimation("Armature|FPS_Pistol_Reload_full");
		nextAnimation = "Armature|FPS_Pistol_Idle";
		nextLooping = true;
		nextAnimTime = 1.6f;
	}
	if (!glfwGetKey(window, GLFW_KEY_R))
		reloadDown = false;

	if (glfwGetKey(window, GLFW_KEY_W) && animator->animationName == "Armature|FPS_Pistol_Idle")
	{
		animator->StartAnimation("Armature|FPS_Pistol_Walk", true);
		nextAnimation = "";
	}
	else if (!glfwGetKey(window, GLFW_KEY_W) && animator->animationName == "Armature|FPS_Pistol_Walk")
	{
		animator->StartAnimation("Armature|FPS_Pistol_Idle", true);
	}


	// Look aim testing
	glm::vec2 mouseDelta = Input::GetMouseDelta();
	rotationX -= mouseDelta.y * 0.1f;
	rotationY -= mouseDelta.x * 0.1f;
	mainObject->eulerRotation = { rotationX, rotationY, 0 };
	mainObject->RecalculateTransforms();
}

void ComponentFPSTest::OnParentChange()
{
	window = Window::Get()->GetGLFWwindow();

	Component* component = componentParent->GetComponent(Component_Animator);
	if (component)
		animator = static_cast<ComponentAnimator*>(component);

	mainObject = componentParent->parent;
}

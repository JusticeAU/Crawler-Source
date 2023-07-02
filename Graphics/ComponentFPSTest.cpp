#include "ComponentFPSTest.h"
#include "ComponentAnimator.h"
#include "Animation.h"
#include "Object.h"
#include "Input.h"
#include "LogUtils.h"

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
	if(Input::Mouse(0).Down())
	{
		fireDown = true;
		clipRounds -= 1;
		animator->StartAnimation("models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Fire");
		nextAnimation = "models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Idle";
		nextLooping = true;
		nextAnimTime = 0.3f;
	}

	if (Input::Keyboard(GLFW_KEY_R).Down())
	{
		reloadDown = true;
		clipRounds = clipCapacity;
		animator->StartAnimation("models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Reload_full");
		nextAnimation = "models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Idle";
		nextLooping = true;
		nextAnimTime = 1.6f;
	}


	if (Input::Keyboard(GLFW_KEY_W).Pressed() && animator->current->animation->name == "models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Idle")
	{
		animator->BlendToAnimation("models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Walk", 0.25f, 0.0f, true);
		nextAnimation = "";
	}
	
	if (Input::Keyboard(GLFW_KEY_W).Up() && animator->current->animation->name == "models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Walk")
	{
		animator->BlendToAnimation("models/FPSPistol/Armpist.fbxArmature|FPS_Pistol_Idle", 0.25f, 0.0f, true);
		nextAnimation = "";
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

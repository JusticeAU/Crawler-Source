#include "TestApplication.h"
#include "glad.h"
#include "MathUtils.h"

TestApplication::TestApplication()
{
	fromColour = { 0.4, 0.65f, 0.91f, 1.0f };
	toColour = { 0.9f, 0.63f, 0.42f, 1.0f };
}

void TestApplication::Update(float delta)
{
	if (to)
	{
		t += delta;
		if (t > 1.0f)
			to = false;
	}
	else
	{
		t -= delta;
		if (t <= 0.0f)
			to = true;
	}


	glClearColor(
		MathUtils::Lerp(fromColour.r, toColour.r, t),
		MathUtils::Lerp(fromColour.g, toColour.g, t),
		MathUtils::Lerp(fromColour.b, toColour.b, t),
		1);
}

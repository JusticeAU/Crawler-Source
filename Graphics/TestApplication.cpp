#include "TestApplication.h"
#include "Graphics.h"
#include "MathUtils.h"

TestApplication::TestApplication()
{
	colors.push_back(glm::vec4(0.45f, 0.92f, 0.57f, 1.0f)); // green
	colors.push_back(glm::vec4(0.10f, 0.17f, 0.47f, 1.0f)); // dark blue
	colors.push_back(glm::vec4(0.9f, 0.63f, 0.42f, 1.0f)); // orange
	colors.push_back(glm::vec4(0.39, 0.58f, 0.92f, 1.0f)); // CORNFLOWER blue
	colors.push_back(glm::vec4(0.62f, 0.38f, 0.63f, 1.0f)); // purp
	colors.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // black
	shader = new ShaderProgram();
	shader->LoadFromFiles("shaders\passthrough.VERT", "shaders\passthrough.FRAG");
}

void TestApplication::Update(float delta)
{
	t += delta * 0.5f;


	if (t > 2.0f)
	{
		t = 0.0f;
		colorIndex = colorIndex + 1 == colors.size() ? 0 : colorIndex + 1;
		nextColor = colorIndex + 1 == colors.size() ? 0 : colorIndex + 1;
	}

	glClearColor(
		MathUtils::Lerp(colors[colorIndex].r, colors[nextColor].r, glm::min(t, 1.0f)),
		MathUtils::Lerp(colors[colorIndex].g, colors[nextColor].g, glm::min(t, 1.0f)),
		MathUtils::Lerp(colors[colorIndex].b, colors[nextColor].b, glm::min(t, 1.0f)),
		1);
}

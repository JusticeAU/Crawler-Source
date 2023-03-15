#include "TestApplication.h"
#include "Graphics.h"
#include "MathUtils.h"

TestApplication::TestApplication(GLFWwindow* window) : window(window)
{
	colors.push_back(glm::vec4(0.45f, 0.92f, 0.57f, 1.0f)); // green
	colors.push_back(glm::vec4(0.10f, 0.17f, 0.47f, 1.0f)); // dark blue
	colors.push_back(glm::vec4(0.9f, 0.63f, 0.42f, 1.0f)); // orange
	colors.push_back(glm::vec4(0.39, 0.58f, 0.92f, 1.0f)); // CORNFLOWER blue
	colors.push_back(glm::vec4(0.62f, 0.38f, 0.63f, 1.0f)); // purp
	colors.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // black

	float aspect;
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	aspect = width / (float)height;

	camera = new Camera(aspect);
	
	
	shader = new ShaderProgram();
	shader->LoadFromFiles("shaders\\passthrough.VERT", "shaders\\passthrough.FRAG");

	// Open GL Init
	glGenBuffers(1, &bufferID);

	// Enable depth testing (Also need ot make sure we are clearing depth values when calling screen clear)
	glEnable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(someFloats), someFloats, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// These align with the 'layout' keywords in the shader.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

}

void TestApplication::Update(float delta)
{
	// Lerp background for funzies
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

	// Draw triangle
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);

	// again, these align with the layout keyword in the shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

	// Move the cube by an offset
	glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(1, 0, 0));

	// Rotate the cube
	glm::mat4 rotation = glm::rotate(glm::mat4(1), (float)glfwGetTime() * 0.2f, glm::normalize(glm::vec3(0.5f,1,1)));

	// Combine the matricies
	glm::mat4 transform = camera->GetMatrix() * translation * rotation;

	shader->Bind();
	shader->SetMatrixUniform("transformMatrix", transform);

	// type, start, number of verticies
	glDrawArrays(GL_TRIANGLES, 0, sizeof(someFloats) / sizeof(float) * 3);

	//round 2 with different translation
	translation = glm::translate(glm::mat4(1), glm::vec3(-1, 0, 0));
	rotation = glm::rotate(glm::mat4(1), (float)glfwGetTime() * 0.2f, glm::normalize(glm::vec3(-0.5f, -1, -1)));
	transform = camera->GetMatrix() * translation * rotation;
	shader->SetMatrixUniform("transformMatrix", transform);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(someFloats) / sizeof(float) * 3);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera->Move({ 0.05f, 0.0f, 0.0f });
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera->Move({ -0.05f, 0.0f, 0.0f });

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera->Move({ 0.0f, 0.0f, 0.05f });
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera->Move({ 0.0f, 0.0f, -0.05f });



}

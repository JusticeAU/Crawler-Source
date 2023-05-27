#include "Application.h"
#include "LogUtils.h"

int main(void)
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
	app->Run();
}


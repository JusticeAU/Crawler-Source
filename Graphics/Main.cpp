#include "Application.h"
#include "LogUtils.h"

int main(void)
{
	// Construct my actual application
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
	
	app->Run();
}


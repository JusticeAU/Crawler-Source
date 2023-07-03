#include "Application.h"
#include "LogUtils.h"

int main(int argc, char * argv[])
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
	if (argc > 1)
		app->LaunchArgument(argv[1]);
	
	if(!app->developerMode)
		app->InitGame();
	
	app->Run();
}


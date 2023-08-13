#include "Application.h"
#include "LogUtils.h"
#include "DungeonHelpers.h"

int main(int argc, char * argv[])
{
	LogUtils::Log("Constructing Application.");
	Application* app = new Application();
	if (argc > 1)
		app->LaunchArgument(argv[1]);
	
	if(app->s_mode == Application::Mode::Game)
		app->InitGame();
	
	app->Run();
}

